import json
import os
import importlib.resources
from cffi import FFI
import findlibs
from dataclasses import dataclass, field
from datetime import datetime, timedelta, timezone
from itertools import combinations
from typing import IO, Iterator
import warnings
from pathlib import Path
import yaml
from ._version import __version__
from .models import ParameterEntry

try:
    import requests as _requests
except ImportError:
    _requests = None

try:
    import platformdirs as _platformdirs
except ImportError:
    _platformdirs = None

ffi = FFI()


def ffi_encode(data) -> bytes:
    if isinstance(data, bytes):
        return data

    if not isinstance(data, str):
        data = str(data)

    return data.encode(encoding="utf-8", errors="surrogateescape")


def ffi_decode(data: FFI.CData) -> str:
    buf = ffi.string(data)
    if isinstance(buf, str):
        return buf
    else:
        return buf.decode(encoding="utf-8", errors="surrogateescape")


@dataclass(frozen=True)
class ParamIDCandidate:
    """One possible paramid for a shortname, plus the context that selects it.

    Attributes
    ----------
    param_id:
        The candidate numeric parameter ID.
    table:
        GRIB parameter table the id encodes to (e.g. ``128``, ``228``).
    origin:
        WMO originating centre ids associated with this candidate — the full
        list (e.g. ``[0, 34, 98]``), since several centres may share the id.
    access:
        Access categories (e.g. ``["dissemination"]``) — the full list.
    mars_request_context:
        The minimal set of MARS key/value pairs that, when passed as
        ``context=`` to :meth:`ParamDB.shortname_to_param_id`, selects this
        candidate (e.g. ``{"class": "ai"}``). Empty if no baked context is
        recorded or if no key-subset uniquely distinguishes this candidate.
    """

    param_id: int
    table: int
    origin: "list[int]"
    access: "list[str]"
    mars_request_context: dict = field(default_factory=dict)


class AmbiguousParamError(KeyError):
    """Raised when a shortname (given the supplied context) maps to >1 paramid.

    Attributes
    ----------
    shortname:
        The shortname that could not be uniquely resolved.
    candidates:
        Every matching candidate, each carrying the context needed to select
        it, sorted by ``(table, origin, access, mars_request_context)``. The
        caller inspects these and re-calls with a narrowing ``context=``.
    """

    def __init__(self, shortname: str, candidates: "list[ParamIDCandidate]"):
        self.shortname = shortname
        self.candidates = candidates
        ids = [c.param_id for c in candidates]
        super().__init__(
            f"Short name {shortname!r} is ambiguous: candidates {ids}. "
            f"Supply context= to disambiguate (see .candidates for options)."
        )


class MarsRequest:
    def __init__(self, verb: str | None = None, **kwargs):
        """
        Create MetKit MarsRequest object. Parameters and values in
        the request can be specified through kwargs, noting that
        reserved words in Python must be suffixed with "_" e.g. "class_"
        """
        if lib is None:
            raise ImportError(
                "MetKit C library is not available. "
                "MarsRequest requires the MetKit shared library to be installed and functional."
            )
        crequest = ffi.new("metkit_marsrequest_t **")
        lib.metkit_marsrequest_new(crequest)
        self.__request = ffi.gc(crequest[0], lib.metkit_marsrequest_delete)
        if verb is not None:
            lib.metkit_marsrequest_set_verb(self.__request, ffi_encode(verb))
        for param, values in kwargs.items():
            self[param.rstrip("_")] = values

    def ctype(self) -> FFI.CData:
        return self.__request

    def verb(self) -> str:
        cverb = ffi.new("const char **")
        lib.metkit_marsrequest_verb(self.__request, cverb)
        return ffi_decode(cverb[0])

    def expand(self, inherit: bool = True, strict: bool = False) -> "MarsRequest":
        """
        Return expanded request

        Params
        ------
        inherit: bool, if True, populates expanded request with default values
        strict: bool, if True, raise error instead of warning for invalid values

        Returns
        -------
        Request, resulting from expansion
        """
        expanded_request = MarsRequest()
        lib.metkit_marsrequest_expand(
            self.__request, inherit, strict, expanded_request.ctype()
        )
        return expanded_request

    def resolved_param(self) -> int:
        """Return the single resolved ``param`` value as an ``int``.

        Convenience accessor for the shortname -> paramid resolution path:
        after building a request with ``param=<shortname>`` plus context keys
        and calling :meth:`expand`, the C++ ``TypeParam::pass2`` rewrites
        ``param`` to the numeric id. This reads that id back.

        Returns
        -------
        int
            The single resolved paramid.

        Raises
        ------
        KeyError
            If the request has no ``param`` key.
        ValueError
            If ``param`` resolved to zero or more than one value (i.e. the
            context did not collapse to a single id).
        """
        if "param" not in self:
            raise KeyError("Request has no 'param' key to resolve")
        n = self.num_values("param")
        if n != 1:
            values = self["param"]
            raise ValueError(
                f"'param' did not resolve to a single value (got {n}: {values})"
            )
        return int(self["param"])

    def validate(self):
        """
        Check if request is valid against MARS language definition. Does not
        inherit missing parameters.

        Raises
        ------
        Exception if request is incompatible with MARS language definition
        """
        self.expand(False, True)

    def keys(self) -> Iterator[str]:
        """
        Get iterator over parameters in request

        Returns
        -------
        Iterator over parameter names
        """
        it_c = ffi.new("metkit_paramiterator_t **")
        lib.metkit_marsrequest_params(self.__request, it_c)
        it = ffi.gc(it_c[0], lib.metkit_paramiterator_delete)

        while lib.metkit_paramiterator_next(it) == lib.METKIT_ITERATOR_SUCCESS:
            cparam = ffi.new("const char **")
            lib.metkit_paramiterator_current(it, cparam)
            param = ffi_decode(cparam[0])
            yield param

    def num_values(self, param: str) -> int:
        """
        Number of values for parameter

        Params
        ------
        param: parameter name

        Returns
        -------
        int
        """
        cparam = ffi_encode(param)
        count = ffi.new("size_t *", 0)
        lib.metkit_marsrequest_count_values(self.__request, cparam, count)
        return count[0]

    def merge(self, other: "MarsRequest") -> "MarsRequest":
        """
        Merge the values in another request with existing request and returns result as a
        new Request object. Does not modify inputs to merge. Both input requests must contain
        the same values and the resulting request object must be compatible with MARS language
        definition

        Params
        ------
        other: Request, request to merge with self

        Returns
        -------
        Request, containing the result of the merge

        Raises
        ------
        ValueError if parameters in the two requests do not match
        MetKitException if resulting request is not compatible with MARS language definition
        """
        if set(self.keys()) != set(other.keys()):
            raise ValueError("Can not merge requests with different parameters.")
        res = MarsRequest(self.verb(), **{k: v for k, v in self})
        lib.metkit_marsrequest_merge(res.ctype(), other.ctype())
        res.validate()
        return res

    def __iter__(self) -> Iterator[tuple[str, list[str]]]:
        for param in self.keys():
            yield param, self[param]

    def __getitem__(self, param: str) -> str | list[str]:
        nvalues = self.num_values(param)
        values = []
        for index in range(nvalues):
            cvalue = ffi.new("const char **")
            lib.metkit_marsrequest_value(
                self.__request, ffi_encode(param), index, cvalue
            )
            value = ffi_decode(cvalue[0])
            if nvalues == 1:
                return value
            values.append(value)
        return values

    def __contains__(self, param: str) -> bool:
        has = ffi.new("bool *", False)
        lib.metkit_marsrequest_has_param(self.__request, ffi_encode(param), has)
        return has[0]

    def __setitem__(self, param: str, values: int | str | list[str]):
        if isinstance(values, (str, int)):
            values = [values]
        cvals = []
        for value in values:
            if isinstance(value, int):
                value = str(value)
            cvals.append(ffi.new("const char[]", value.encode("ascii")))
        lib.metkit_marsrequest_set(
            self.__request,
            ffi_encode(param),
            ffi.new("const char*[]", cvals),
            len(values),
        )

    def __eq__(self, other: "MarsRequest") -> bool:
        if self.verb() != other.verb():
            return False
        expanded = self.expand()
        other_expanded = other.expand()
        return dict(expanded) == dict(other_expanded)


def parse_mars_request(
    file_or_str: IO | str, strict: bool = False
) -> list[MarsRequest]:
    """
    Function for parsing mars request from file object or string.

    Params
    ------
    file_or_str: string or file-like object, containing mars request
    strict: bool, whether to raise error or warning when request is not compatible with
    MARS language definition. In the case of warning, when False, the incompatible
    parameters are unset from the request.

    Returns
    -------
    list of Request
    """
    if lib is None:
        raise ImportError(
            "MetKit C library is not available. "
            "parse_mars_request() requires the MetKit shared library to be installed and functional."
        )
    crequest_iter = ffi.new("metkit_requestiterator_t **")

    if isinstance(file_or_str, str):
        lib.metkit_parse_marsrequests(ffi_encode(file_or_str), crequest_iter, strict)
    else:
        lib.metkit_parse_marsrequests(
            ffi_encode(file_or_str.read()), crequest_iter, strict
        )
    request_iter = ffi.gc(crequest_iter[0], lib.metkit_requestiterator_delete)

    requests = []
    while lib.metkit_requestiterator_next(request_iter) == lib.METKIT_ITERATOR_SUCCESS:
        new_request = MarsRequest()
        lib.metkit_requestiterator_current(request_iter, new_request.ctype())
        requests.append(new_request)

    return requests


class MetKitException(RuntimeError):
    """Raised when MetKit library throws exception"""

    pass


class CFFIModuleLoadFailed(ImportError):
    """Raised when the shared library fails to load"""

    pass


class PatchedLib:
    """
    Patch a CFFI library with error handling

    Finds the header file associated with the MetKit C API and parses it,
    loads the shared library, and patches the accessors with
    automatic python-C error handling.
    """

    def __init__(self):
        libName = findlibs.find("metkit")

        if libName is None:
            raise RuntimeError("MetKit library not found")

        ffi.cdef(self.__read_header())
        try:
            self.__lib = ffi.dlopen(libName)
        except OSError as e:
            raise CFFIModuleLoadFailed(
                f"MetKit shared library found at '{libName}' but could not be "
                f"loaded ({e}). It is likely stale or incompatible (e.g. an "
                "eckit/metkit ABI mismatch). ParamDB remains usable."
            ) from e

        # All of the executable members of the CFFI-loaded library are functions in the MetKit
        # C API. These should be wrapped with the correct error handling. Otherwise forward
        # these on directly.

        missing_symbols = []
        for f in dir(self.__lib):
            try:
                attr = getattr(self.__lib, f)
                setattr(
                    self, f, self.__check_error(attr, f) if callable(attr) else attr
                )
            except Exception:
                missing_symbols.append(f)

        # Initialise the library, and set it up for python-appropriate behaviour

        try:
            self.metkit_initialise()
        except AttributeError:
            raise CFFIModuleLoadFailed(
                f"MetKit shared library loaded from '{libName}' but required symbols are "
                f"missing ({len(missing_symbols)} of {len(dir(self.__lib))} failed). "
                "The library is stale or incompatible with this version of pymetkit."
            )

        # Check the library version

        versionstr = ffi.string(self.metkit_version()).decode("utf-8")
        if versionstr != __version__:
            warnings.warn(
                f"Metkit library version {versionstr} does not match python version {__version__}"
            )

    def __read_header(self):
        with open(os.path.join(os.path.dirname(__file__), "metkit_c.h"), "r") as f:
            return f.read()

    def __check_error(self, fn, name: str):
        """
        If calls into the MetKit library return errors, ensure that they get
        detected and reported by throwing an appropriate python exception.
        """

        def wrapped_fn(*args, **kwargs):
            # debug
            retval = fn(*args, **kwargs)

            # Some functions dont return error codes. Ignore these.
            if name in ["metkit_version", "metkit_git_sha1"]:
                return retval

            # error codes:
            if retval not in (
                self.__lib.METKIT_SUCCESS,
                self.__lib.METKIT_ITERATOR_SUCCESS,
                self.__lib.METKIT_ITERATOR_COMPLETE,
            ):
                err = ffi_decode(self.__lib.metkit_get_error_string(retval))
                msg = "Error in function '{}': {}".format(name, err)
                raise MetKitException(msg)
            return retval

        return wrapped_fn


class ParamDB:
    """
    Parameter database providing metadata lookup for ECMWF parameters.

    Supports both online mode (fetching from the ECMWF parameter database API)
    and offline mode (loading from a bundled YAML file).

    When using ``mode="online"`` a local JSON cache is maintained so that
    repeated instantiations within the TTL window do not make a new HTTP
    request.  The cache is stored under the OS user-cache directory
    (e.g. ``~/.cache/pymetkit/`` on Linux, ``~/Library/Caches/pymetkit/``
    on macOS) using the fixed filename defined by ``_CACHE_FILENAME``.

    Shortname collision resolution
    --------------------------------
    Some short names (e.g. ``t``, ``tp``, ``u``) are reused across different
    GRIB parameter tables and originating centres.  When no context is given
    the default resolution priority is:

    1. Prefer parameters with ``"dissemination"`` in their ``access_ids``.
    2. Among those, prefer parameters whose ``origin_ids`` include an origin
       from ``_DEFAULT_ORIGIN_PREFERENCE`` (tried in order).
    3. Fall back to the lowest param ID.

    Pass ``table=``, ``origin=``, or ``access=`` to
    :meth:`shortname_to_param_id` / :meth:`shortname_to_longname` to override
    this behaviour explicitly.
    """

    _API_URL = "https://codes.ecmwf.int/parameter-database/api/v1/param/"

    #: File name written inside the platform cache directory.
    _CACHE_FILENAME = "paramdb_online_cache.json"

    #: Default time-to-live for the online cache.
    _DEFAULT_CACHE_TTL = timedelta(hours=1)

    #: Default HTTP request timeout in seconds for online API calls.
    _REQUEST_TIMEOUT = 30

    #: Ordered list of WMO originating centre IDs tried when resolving a
    #: colliding shortname with no explicit ``origin=`` context.
    #: 98 = ECMWF, 0 = WMO.
    _DEFAULT_ORIGIN_PREFERENCE: list[int] = [98, 0]

    def __init__(
        self,
        mode: str = "offline",
        cache_ttl: "timedelta | None" = None,
        cache_path: "Path | str | None" = None,
        yaml_path: "Path | str | None" = None,
    ):
        """
        Initialise the parameter database.

        The underlying data is loaded **lazily** — no file I/O or network
        request is made until the first lookup method is called.  This makes
        instantiation cheap and safe to do at import time or inside hot paths.

        Parameters
        ----------
        mode : str
            Either ``"online"`` (fetch from the ECMWF API) or
            ``"offline"`` (load from a YAML file).
        cache_ttl : datetime.timedelta, optional
            How long a previously fetched online result may be reused before a
            fresh HTTP request is made.  Defaults to 1 hour.  Only relevant
            when ``mode="online"``.  Pass ``timedelta(0)`` to disable caching
            entirely (always fetch).
        cache_path : Path or str, optional
            Directory in which to store the cache file.  Defaults to the
            OS-appropriate user cache directory (requires ``platformdirs``).
            Only relevant when ``mode="online"``.
        yaml_path : Path or str, optional
            Path to a custom YAML file to load instead of the bundled
            ``parameter_metadata.yaml``.  The file must be a YAML list where
            each entry contains at minimum an ``id`` (integer), a short name
            (``shortname`` / ``shortName`` / ``short_name``), and a long name
            (``longname`` / ``longName`` / ``long_name`` / ``name``).
            Only valid with ``mode="offline"``; raises ``ValueError`` if
            combined with ``mode="online"``.
        """
        if mode not in ("online", "offline"):
            raise ValueError(f"mode must be 'online' or 'offline', got '{mode}'")

        if yaml_path is not None and mode == "online":
            raise ValueError(
                "yaml_path cannot be used with mode='online'. "
                "Use mode='offline' to load from a YAML file."
            )

        # Validate and resolve cache_ttl up-front so callers get a TypeError at
        # construction time rather than at first lookup.
        if mode == "online":
            effective_ttl = self._DEFAULT_CACHE_TTL if cache_ttl is None else cache_ttl
            if not isinstance(effective_ttl, timedelta):
                raise TypeError(
                    f"cache_ttl must be a datetime.timedelta, got {type(effective_ttl).__name__!r}"
                )
        else:
            effective_ttl = None

        # Store load parameters for deferred use.
        self._mode = mode
        self._effective_cache_ttl = effective_ttl
        self._cache_path = cache_path
        self._yaml_path = yaml_path

        # Lookup indices — populated on first access via _ensure_loaded().
        self._by_id: dict[int, dict] = {}
        self._by_shortname: dict[str, dict] = {}
        self._by_shortname_all: dict[str, list[dict]] = {}
        self._by_longname: dict[str, dict] = {}
        self._loaded: bool = False
        # Memo for the C++ expand oracle: (shortname, sorted-context-tuple) -> ids.
        self._ctx_cache: dict = {}

    # ------------------------------------------------------------------
    # Private helpers
    # ------------------------------------------------------------------

    @staticmethod
    def _table_from_id(param_id: int) -> int:
        """Decode the GRIB parameter table number from an encoded param ID.

        The encoding scheme mirrors the C++ ``Param::paramId()`` logic:

        * IDs 1–999          → table 128 (classic ECMWF, table prefix suppressed)
        * IDs 1 000–999 999  → ``table * 1000 + param``  (e.g. 228228 → table 228)
        * IDs ≥ 1 000 000    → ``center * 1_000_000 + table * 1000 + param``
          (e.g. 7001292 → center 7, table 1)
        """
        if param_id < 1_000:
            return 128
        elif param_id < 1_000_000:
            return param_id // 1_000
        else:
            return (param_id % 1_000_000) // 1_000

    @staticmethod
    def _center_from_id(param_id: int) -> "int | None":
        """Decode the originating WMO center from an encoded param ID.

        Returns ``None`` for IDs below 1 000 000 (ECMWF-local parameters).
        For IDs ≥ 1 000 000 the center is ``param_id // 1_000_000``.
        """
        if param_id >= 1_000_000:
            return param_id // 1_000_000
        return None

    def _resolve_shortname_with_context(
        self,
        shortname: str,
        table: "int | None" = None,
        origin: "int | None" = None,
        access: "str | None" = None,
    ) -> dict:
        """Return the best-matching entry for *shortname* given optional context.

        Parameters
        ----------
        shortname:
            The ECMWF short name to look up.
        table:
            GRIB parameter table number (e.g. ``128`` for classic ECMWF,
            ``140`` for ocean waves, ``228`` for "Standard 2").  When
            provided, only candidates whose encoded param ID belongs to this
            table are considered.
        origin:
            WMO originating centre ID (e.g. ``98`` for ECMWF, ``0`` for WMO,
            ``7`` for NCEP).  When provided, only candidates whose
            ``origin_ids`` list includes this value are considered.
        access:
            Access category string (e.g. ``"dissemination"``).  When
            provided, only candidates whose ``access_ids`` list includes this
            value are considered.

        Returns
        -------
        dict
            The matched parameter metadata entry.

        Raises
        ------
        KeyError
            If *shortname* is not found, or if no candidate matches the
            supplied context.
        """
        if shortname not in self._by_shortname_all:
            raise KeyError(f"Short name {shortname!r} not found in database")

        candidates = self._by_shortname_all[shortname]

        # --- Explicit context filters (hard constraints) ---
        if table is not None:
            candidates = [
                e for e in candidates if self._table_from_id(e["id"]) == table
            ]
        if origin is not None:
            candidates = [
                e for e in candidates if origin in e.get("origin_ids", [])
            ]
        if access is not None:
            candidates = [
                e for e in candidates if access in e.get("access_ids", [])
            ]

        if not candidates:
            ctx_parts = []
            if table is not None:
                ctx_parts.append(f"table={table}")
            if origin is not None:
                ctx_parts.append(f"origin={origin}")
            if access is not None:
                ctx_parts.append(f"access={access!r}")
            raise KeyError(
                f"Short name {shortname!r} not found for context "
                f"{', '.join(ctx_parts)}"
            )

        # If any explicit context was given, return the lowest-id match among
        # the filtered set and skip the default priority logic.
        if table is not None or origin is not None or access is not None:
            return min(candidates, key=lambda e: e["id"])

        # --- Default priority logic (no explicit context) ---
        # 1. Prefer dissemination parameters.
        dissem = [e for e in candidates if "dissemination" in e.get("access_ids", [])]
        pool = dissem if dissem else candidates

        # 2. Among the pool, prefer origins in _DEFAULT_ORIGIN_PREFERENCE order.
        for preferred_origin in self._DEFAULT_ORIGIN_PREFERENCE:
            origin_match = [
                e for e in pool if preferred_origin in e.get("origin_ids", [])
            ]
            if origin_match:
                return min(origin_match, key=lambda e: e["id"])

        # 3. Fall back to lowest id.
        return min(pool, key=lambda e: e["id"])

    # ------------------------------------------------------------------
    # Context-aware resolution helpers (v2 API)
    # ------------------------------------------------------------------

    def _context_resolved_ids(self, shortname: str, context: dict) -> "set[int] | None":
        """Resolve *shortname* + *context* to paramid(s) via the C++ engine.

        Builds ``MarsRequest(param=shortname, **context)``, expands it (which
        runs ``TypeParam::pass2`` and fills MARS defaults for unspecified keys),
        and reads back the resolved ``param`` value(s). Results are memoised on
        ``self._ctx_cache`` since each ``expand`` is a comparatively expensive
        (~ms) C++ round-trip and the minimal-context search repeats queries.

        The C++ library reads its language files (``params.yaml`` etc.) from
        ``~metkit/share/metkit`` — set ``METKIT_HOME`` to this repository root
        to resolve against the in-repo ``share/metkit`` data.

        Returns
        -------
        set[int] | None
            The set of resolved numeric ids, or ``None`` if the MetKit C
            library is unavailable (caller should fall back to baked contexts).
        """
        if lib is None:
            return None
        cache_key = (shortname, tuple(sorted((str(k).rstrip("_"), str(v)) for k, v in context.items())))
        cached = self._ctx_cache.get(cache_key)
        if cached is not None:
            return cached
        req = MarsRequest(verb="retrieve")
        req["param"] = shortname
        for key, value in context.items():
            req[key.rstrip("_")] = value
        expanded = req.expand()
        if "param" not in expanded:
            self._ctx_cache[cache_key] = set()
            return set()
        values = expanded["param"]
        if isinstance(values, str):
            values = [values]
        resolved: set[int] = set()
        for v in values:
            try:
                resolved.add(int(v))
            except (TypeError, ValueError):
                continue
        self._ctx_cache[cache_key] = resolved
        return resolved

    @staticmethod
    def _entry_matches_context(entry: dict, context: dict) -> bool:
        """Offline fallback: does any baked context of *entry* satisfy *context*?

        Used only when the C++ library is unavailable. An entry matches when at
        least one of its ``mars_request_context`` dicts contains every
        ``key == value`` pair in *context*.
        """
        wanted = {k.rstrip("_"): str(v) for k, v in context.items()}
        for baked in entry.get("mars_request_context", []):
            if all(str(baked.get(k)) == v for k, v in wanted.items()):
                return True
        return False

    def _minimal_distinguishing_context(
        self, entry: dict, siblings: "list[dict]", shortname: str
    ) -> dict:
        """Return the smallest MARS key-subset that selects *entry* over siblings.

        Searches the key-subsets of *entry*'s baked ``mars_request_context``
        dicts for the smallest subset that uniquely selects this id.

        When the MetKit C library is available, ``expand`` is used as the
        authoritative oracle: a subset qualifies only if
        ``MarsRequest(param=shortname, **subset).expand()`` resolves to exactly
        this id. This guarantees the advertised context actually round-trips
        (bare ``{"class": "ai"}`` is rejected for ``tp`` because ``expand``'s
        inherited defaults resolve it to 228, not 228228).

        Offline (no library), it falls back to a set-membership heuristic
        against the baked contexts of the sibling candidates.

        Returns ``{}`` if no subset uniquely distinguishes *entry* (residual
        ambiguity) or if *entry* has no baked context.
        """
        entry_id = int(entry["id"])
        use_oracle = lib is not None

        other_contexts: list[dict] = []
        if not use_oracle:
            for sib in siblings:
                if sib.get("id") == entry_id:
                    continue
                other_contexts.extend(sib.get("mars_request_context", []))

        def selects(subctx: dict) -> bool:
            if use_oracle:
                return self._context_resolved_ids(shortname, subctx) == {entry_id}
            return not any(
                all(str(o.get(k)) == str(v) for k, v in subctx.items())
                for o in other_contexts
            )

        best: "dict | None" = None
        for ctx in entry.get("mars_request_context", []):
            keys = sorted(ctx.keys())
            for size in range(1, len(keys) + 1):
                if best is not None and size >= len(best):
                    break
                found_at_size = None
                for combo in combinations(keys, size):
                    subctx = {k: ctx[k] for k in combo}
                    if selects(subctx):
                        found_at_size = subctx
                        break
                if found_at_size is not None:
                    best = found_at_size
                    break
        return best or {}

    def _make_candidate(
        self,
        entry: dict,
        siblings: "list[dict]",
        shortname: str,
        minimal: bool = True,
    ) -> ParamIDCandidate:
        """Build a :class:`ParamIDCandidate` from a metadata *entry*.

        When *minimal* is ``False`` the (expensive, expand-oracle) minimal
        distinguishing context is skipped and left empty — used when the entry
        is already unambiguous so no disambiguating context is required.
        """
        param_id = int(entry["id"])
        ctx = (
            self._minimal_distinguishing_context(entry, siblings, shortname)
            if minimal
            else {}
        )
        return ParamIDCandidate(
            param_id=param_id,
            table=self._table_from_id(param_id),
            origin=list(entry.get("origin_ids", [])),
            access=list(entry.get("access_ids", [])),
            mars_request_context=ctx,
        )

    @staticmethod
    def _candidate_sort_key(cand: ParamIDCandidate) -> tuple:
        return (
            cand.table,
            tuple(sorted(cand.origin)),
            tuple(sorted(cand.access)),
            tuple(sorted((str(k), str(v)) for k, v in cand.mars_request_context.items())),
            cand.param_id,
        )

    @staticmethod
    def _normalise(raw: dict) -> dict:
        """Return a normalised and validated parameter dict with canonical key names.

        Uses :class:`~pymetkit.models.ParameterEntry` for field coercion and
        validation.  Extra keys present in *raw* (e.g. ``"url"``, ``"units_id"``)
        are preserved in the returned dict so no information is discarded.
        """
        entry = ParameterEntry.model_validate(raw)
        # model_dump preserves extra fields captured via ``extra="allow"``.
        # by_alias=True keeps MARS ``class`` (not the Python-safe ``class_``).
        return entry.model_dump(by_alias=True)

    def _index(self, entry: dict) -> None:
        """Insert a normalised entry into the internal lookup dicts."""
        param_id = entry.get("id")
        shortname = entry.get("shortname")
        longname = entry.get("longname")

        if param_id is not None:
            self._by_id[int(param_id)] = entry
        if shortname is not None:
            sn = str(shortname)
            # first-write-wins: entries are loaded in ascending id order, so the
            # lowest (most canonical) id wins for the default shortname lookup.
            if sn not in self._by_shortname:
                self._by_shortname[sn] = entry
            # _by_shortname_all keeps every candidate for context-aware lookup.
            self._by_shortname_all.setdefault(sn, []).append(entry)
        if longname is not None:
            ln = str(longname)
            if ln not in self._by_longname:
                self._by_longname[ln] = entry

    def _ensure_loaded(self) -> None:
        """Load parameter data on first access (lazy initialisation).

        Subsequent calls are no-ops — once ``_loaded`` is ``True`` the method
        returns immediately without any I/O.
        """
        if self._loaded:
            return
        if self._mode == "online":
            self._load_online(
                cache_ttl=self._effective_cache_ttl,
                cache_path=self._cache_path,
            )
        else:
            self._load_offline(yaml_path=self._yaml_path)
        self._loaded = True

    def _load_online(
        self, cache_ttl: timedelta, cache_path: "Path | str | None"
    ) -> None:
        if _requests is None:
            raise ImportError(
                "The 'requests' package is required for online mode. "
                "Install it with: pip install requests"
            )

        # Try the cache first (unless TTL is zero).
        # Cache stores already-normalised entries — skip Pydantic on hit.
        if cache_ttl > timedelta(0):
            cached = self._read_cache(cache_path, cache_ttl)
            if cached is not None:
                for entry in cached:
                    self._index(entry)
                return

        # Fetch from the API
        response = _requests.get(self._API_URL, timeout=self._REQUEST_TIMEOUT)
        response.raise_for_status()
        params = response.json()

        # Normalise once (coerces API field names to canonical keys)
        normalised = [self._normalise(raw) for raw in params]

        # Persist normalised entries to cache (best-effort)
        if cache_ttl > timedelta(0):
            self._write_cache(normalised, cache_path)

        for entry in normalised:
            self._index(entry)

    def _load_offline(self, yaml_path: "Path | str | None" = None) -> None:
        if yaml_path is not None:
            resolved = Path(yaml_path)
            if not resolved.exists():
                raise FileNotFoundError(f"Custom YAML file not found: {resolved}")
            # Custom YAML: run Pydantic validation to normalise aliases/types
            with resolved.open("r") as fh:
                params = yaml.safe_load(fh)
            for raw in params:
                self._index(self._normalise(raw))
        else:
            # Bundled data: prefer JSON (fast) over YAML (slow).
            # Both contain identical data — JSON loads ~10-50× faster.
            json_path = self._find_offline_json()
            if json_path is not None:
                with json_path.open("r") as fh:
                    params = json.load(fh)
            else:
                resolved = self._find_offline_yaml()
                with resolved.open("r") as fh:
                    params = yaml.safe_load(fh)
            for raw in params:
                self._index(raw)

    @staticmethod
    def _find_offline_yaml() -> Path:
        """Locate ``parameter_metadata.yaml``, searching in order:

        1. Via ``importlib.resources`` from the installed package (reliable in
           both regular installs and zip-safe wheels).
        2. Next to this module file (editable / development install layout).
        3. ``<repo_root>/share/metkit/`` (development tree layout after the
           YAML files were moved out of the Python package directory).
        """
        # Candidate 1: importlib.resources (correct path for installed packages)
        try:
            ref = importlib.resources.files("pymetkit").joinpath(
                "parameter_metadata.yaml"
            )
            # Materialise to a real filesystem path so callers can open() it.
            with importlib.resources.as_file(ref) as p:
                if p.exists():
                    return p
        except (FileNotFoundError, TypeError, AttributeError):
            pass

        # Candidates 2 & 3: filesystem heuristics (dev tree / editable install)
        candidates = [
            Path(__file__).parent / "parameter_metadata.yaml",
            Path(__file__).parents[4] / "share" / "metkit" / "parameter_metadata.yaml",
        ]
        for path in candidates:
            if path.exists():
                return path
        raise FileNotFoundError(
            "parameter_metadata.yaml not found. Searched:\n"
            + "\n".join(f"  {p}" for p in candidates)
        )

    @staticmethod
    def _find_offline_json() -> "Path | None":
        """Locate ``parameter_metadata.json`` (fast-load format).

        Returns ``None`` if the JSON file is not found — caller should fall
        back to the YAML file.  Searches the same locations as
        :meth:`_find_offline_yaml`.
        """
        # Candidate 1: importlib.resources
        try:
            ref = importlib.resources.files("pymetkit").joinpath(
                "parameter_metadata.json"
            )
            with importlib.resources.as_file(ref) as p:
                if p.exists():
                    return p
        except (FileNotFoundError, TypeError, AttributeError):
            pass

        # Candidates 2 & 3: filesystem heuristics
        candidates = [
            Path(__file__).parent / "parameter_metadata.json",
            Path(__file__).parents[4] / "share" / "metkit" / "parameter_metadata.json",
        ]
        for path in candidates:
            if path.exists():
                return path
        return None

    # ------------------------------------------------------------------
    # Cache helpers (online mode only)
    # ------------------------------------------------------------------

    def _resolve_cache_dir(self, cache_path: "Path | str | None") -> "Path | None":
        """Return the directory to use for the cache file, or None if unavailable."""
        if cache_path is not None:
            return Path(cache_path)
        if _platformdirs is not None:
            return Path(_platformdirs.user_cache_dir("pymetkit"))
        return None

    def _cache_file(self, cache_path: "Path | str | None") -> "Path | None":
        """Return the full path to the cache file, or None if no cache dir is available."""
        cache_dir = self._resolve_cache_dir(cache_path)
        if cache_dir is None:
            return None
        return cache_dir / self._CACHE_FILENAME

    def _read_cache(
        self, cache_path: "Path | str | None", cache_ttl: timedelta
    ) -> "list | None":
        """
        Return the cached parameter list if it exists and is still fresh,
        otherwise return None.
        """
        cache_file = self._cache_file(cache_path)
        if cache_file is None or not cache_file.exists():
            return None
        try:
            payload = json.loads(cache_file.read_text(encoding="utf-8"))
            fetched_at = datetime.fromisoformat(payload["fetched_at"])
            # Ensure both datetimes are timezone-aware for comparison
            now = datetime.now(tz=timezone.utc)
            if fetched_at.tzinfo is None:
                fetched_at = fetched_at.replace(tzinfo=timezone.utc)
            if (now - fetched_at) <= cache_ttl:
                return payload["params"]
        except Exception:
            # Corrupt or unreadable cache — treat as a miss
            pass
        return None

    def _write_cache(self, params: list, cache_path: "Path | str | None") -> None:
        """Persist *params* to the cache file (best-effort; errors are silenced)."""
        cache_file = self._cache_file(cache_path)
        if cache_file is None:
            return
        try:
            cache_file.parent.mkdir(parents=True, exist_ok=True)
            payload = {
                "fetched_at": datetime.now(tz=timezone.utc).isoformat(),
                "params": params,
            }
            cache_file.write_text(json.dumps(payload), encoding="utf-8")
        except Exception:
            pass

    def _resolve(self, identifier: "int | str") -> dict:
        """Resolve *identifier* (param_id, shortname, or longname) to a metadata dict."""
        # Try as integer param_id first
        if isinstance(identifier, int):
            if identifier in self._by_id:
                return self._by_id[identifier]
            raise KeyError(f"Parameter with id {identifier!r} not found in database")
        # For strings: try coercing to int, then shortname, then longname
        if isinstance(identifier, str):
            try:
                int_id = int(identifier)
                if int_id in self._by_id:
                    return self._by_id[int_id]
            except ValueError:
                pass
            if identifier in self._by_shortname:
                return self._by_shortname[identifier]
            if identifier in self._by_longname:
                return self._by_longname[identifier]
        raise KeyError(f"Parameter {identifier!r} not found in database")

    # ------------------------------------------------------------------
    # Conversion methods
    # ------------------------------------------------------------------

    def shortname_to_longname(
        self,
        shortname: str,
        table: "int | None" = None,
        origin: "int | None" = None,
        access: "str | None" = None,
    ) -> str:
        """Return the long name for *shortname*.

        Parameters
        ----------
        shortname:
            ECMWF short name (e.g. ``"t"``, ``"tp"``).
        table:
            Optional GRIB parameter table number to disambiguate collisions
            (e.g. ``128`` for classic ECMWF, ``140`` for ocean waves).
        origin:
            Optional WMO originating centre ID (e.g. ``98`` for ECMWF,
            ``0`` for WMO, ``7`` for NCEP).
        access:
            Optional access category filter (e.g. ``"dissemination"``).
        """
        self._ensure_loaded()
        return self._resolve_shortname_with_context(
            shortname, table, origin, access
        )["longname"]

    def longname_to_shortname(self, longname: str) -> str:
        self._ensure_loaded()
        if longname not in self._by_longname:
            raise KeyError(f"Long name {longname!r} not found in database")
        return self._by_longname[longname]["shortname"]

    def shortname_to_param_id_candidates(
        self,
        shortname: str,
        context: "dict | None" = None,
        *,
        table: "int | None" = None,
        origin: "int | None" = None,
        access: "str | None" = None,
    ) -> "list[ParamIDCandidate]":
        """Return all candidate paramids for *shortname*, each with its context.

        The programmatic counterpart to :class:`AmbiguousParamError`: it returns
        the candidate + context information as a normal value, so callers can
        inspect the options and then call :meth:`shortname_to_param_id` with the
        narrowing ``context=`` (or ``table``/``origin``/``access``) they want.

        Two independent narrowing mechanisms are available and may be combined:

        * ``context`` — a dict of MARS keys resolved via the C++ ``expand``
          engine (authoritative, cycle-correct). When the MetKit C library is
          unavailable, the baked ``mars_request_context`` metadata is used as a
          fallback.
        * ``table`` / ``origin`` / ``access`` — direct hard filters on the
          candidate metadata, applied WITHOUT constructing a MARS request.

        Parameters
        ----------
        shortname:
            ECMWF short name (e.g. ``"t"``, ``"tp"``).
        context:
            Optional dict of MARS keys used to pre-narrow the candidate set
            (e.g. ``{"class": "ai"}``). When omitted, all candidates surviving
            the hard filters are returned.
        table:
            Optional hard filter — GRIB parameter table number.
        origin:
            Optional hard filter — WMO originating centre id (membership).
        access:
            Optional hard filter — access category string (membership).

        Returns
        -------
        list[ParamIDCandidate]
            Every matching candidate, sorted by
            ``(table, origin, access, mars_request_context, param_id)``. Length
            1 means the shortname (given any supplied context/filters) is
            unambiguous.

        Raises
        ------
        KeyError
            If *shortname* is unknown, or no candidate survives the filters.
        """
        self._ensure_loaded()
        entries, siblings = self._filter_shortname_entries(
            shortname, context, table=table, origin=origin, access=access
        )
        # A single surviving candidate is unambiguous: no distinguishing
        # context is needed, so skip the (expensive) expand-oracle computation.
        if len(entries) == 1:
            return [self._make_candidate(entries[0], siblings, shortname, minimal=False)]
        candidates = [
            self._make_candidate(e, siblings, shortname) for e in entries
        ]
        candidates.sort(key=self._candidate_sort_key)
        return candidates

    def _filter_shortname_entries(
        self,
        shortname: str,
        context: "dict | None" = None,
        *,
        table: "int | None" = None,
        origin: "int | None" = None,
        access: "str | None" = None,
    ) -> "tuple[list[dict], list[dict]]":
        """Return ``(surviving_entries, all_siblings)`` for *shortname*.

        Applies the ``table``/``origin``/``access`` hard filters and the MARS
        ``context`` filter (via the C++ ``expand`` engine, or the baked-context
        offline fallback). Does NOT compute minimal contexts — that expensive
        step is deferred to :meth:`_make_candidate`. Raises ``KeyError`` if the
        shortname is unknown or no entry survives the filters.
        """
        if shortname not in self._by_shortname_all:
            raise KeyError(f"Short name {shortname!r} not found in database")

        siblings = self._by_shortname_all[shortname]
        entries = list(siblings)

        # --- Hard metadata filters -----------------------------------------
        if table is not None:
            entries = [e for e in entries if self._table_from_id(e["id"]) == table]
        if origin is not None:
            entries = [e for e in entries if origin in e.get("origin_ids", [])]
        if access is not None:
            entries = [e for e in entries if access in e.get("access_ids", [])]

        # --- MARS context filter (C++ expand, with offline fallback) -------
        if context:
            resolved = self._context_resolved_ids(shortname, context)
            if resolved is not None:
                entries = [e for e in entries if int(e["id"]) in resolved]
            else:
                entries = [
                    e for e in entries if self._entry_matches_context(e, context)
                ]

        if not entries:
            ctx_parts = []
            if context:
                ctx_parts.append(f"context={context!r}")
            if table is not None:
                ctx_parts.append(f"table={table}")
            if origin is not None:
                ctx_parts.append(f"origin={origin}")
            if access is not None:
                ctx_parts.append(f"access={access!r}")
            raise KeyError(
                f"Short name {shortname!r} not found for "
                f"{', '.join(ctx_parts) or 'the given filters'}"
            )

        return entries, siblings

    def shortname_to_param_id(
        self,
        shortname: str,
        context: "dict | None" = None,
        *,
        table: "int | None" = None,
        origin: "int | None" = None,
        access: "str | None" = None,
    ) -> int:
        """Return the single param ID for *shortname*, given optional context.

        Ambiguity is never resolved by guessing. When more than one candidate
        remains after applying ``context`` and the ``table``/``origin``/
        ``access`` filters, :class:`AmbiguousParamError` is raised; its
        ``.candidates`` attribute lists every remaining id with the context
        needed to select it.

        Parameters
        ----------
        shortname:
            ECMWF short name (e.g. ``"t"``, ``"tp"``).
        context:
            Optional dict of MARS keys resolved via the C++ ``expand`` engine
            (e.g. ``{"class": "ai"}``). Partial context is usually sufficient —
            ``expand`` fills defaults for unspecified keys.
        table:
            Optional hard filter — GRIB parameter table number.
        origin:
            Optional hard filter — WMO originating centre id (membership).
        access:
            Optional hard filter — access category string (membership).

        Returns
        -------
        int
            The uniquely resolved paramid.

        Raises
        ------
        KeyError
            If *shortname* is unknown, or no candidate survives the filters.
        AmbiguousParamError
            If more than one candidate remains after applying context/filters.
        """
        self._ensure_loaded()
        entries, siblings = self._filter_shortname_entries(
            shortname, context, table=table, origin=origin, access=access
        )
        if len(entries) == 1:
            return int(entries[0]["id"])
        # Ambiguous: only now pay the cost of computing per-candidate
        # distinguishing contexts for the error payload.
        candidates = [
            self._make_candidate(e, siblings, shortname) for e in entries
        ]
        candidates.sort(key=self._candidate_sort_key)
        raise AmbiguousParamError(shortname, candidates)


    def param_id_to_shortname(self, param_id: int) -> str:
        self._ensure_loaded()
        if param_id not in self._by_id:
            raise KeyError(f"Parameter id {param_id!r} not found in database")
        return self._by_id[param_id]["shortname"]

    @staticmethod
    def _param_context_from_cpp(param_id: int) -> "list[dict] | None":
        """Return MARS contexts for *param_id* from the C++ layer, if available.

        Placeholder for the future ``metkit_param_context`` C API (outcomes §4).
        That hook does not exist yet, so this always returns ``None`` to signal
        "not available", causing :meth:`param_id_to_context` to fall back to the
        precomputed YAML data. Once the C API lands, implement it here and the
        public method will transparently prefer it.
        """
        return None

    def param_id_to_context(self, param_id: int) -> "list[dict]":
        """Return the MARS-key contexts in which *param_id* is valid.

        Each context is a dict of MARS keys (e.g.
        ``{"class": "ai", "stream": "enfo", "type": "cf", "levtype": "sfc"}``)
        drawn from the authoritative ``params.yaml`` map. These are the raw
        contexts used to disambiguate shortname collisions.

        Resolution order:

        1. The C++ layer (``metkit_param_context``) when available — see
           :meth:`_param_context_from_cpp`. Not yet implemented.
        2. Fallback: the precomputed ``mars_request_context`` field baked into
           the bundled parameter metadata.

        Parameters
        ----------
        param_id:
            Numeric parameter id.

        Returns
        -------
        list[dict]
            Zero or more MARS context dicts. Empty if the id has no recorded
            context (e.g. not referenced in ``params.yaml``).

        Raises
        ------
        KeyError
            If *param_id* is not in the database.
        """
        self._ensure_loaded()
        if param_id not in self._by_id:
            raise KeyError(f"Parameter id {param_id!r} not found in database")

        # Prefer the authoritative C++ source once it exists.
        cpp = self._param_context_from_cpp(param_id)
        if cpp is not None:
            return cpp

        # Fallback: precomputed contexts from the bundled data.
        return list(self._by_id[param_id].get("mars_request_context", []))

    def longname_to_param_id(self, longname: str) -> int:
        self._ensure_loaded()
        if longname not in self._by_longname:
            raise KeyError(f"Long name {longname!r} not found in database")
        return int(self._by_longname[longname]["id"])

    def param_id_to_longname(self, param_id: int) -> str:
        self._ensure_loaded()
        if param_id not in self._by_id:
            raise KeyError(f"Parameter id {param_id!r} not found in database")
        return self._by_id[param_id]["longname"]

    # ------------------------------------------------------------------
    # Metadata retrieval methods
    # ------------------------------------------------------------------

    def get_metadata(self, identifier: "int | str") -> dict:
        """
        Return the full metadata dictionary for a parameter.

        Parameters
        ----------
        identifier : int or str
            A param ID (int), shortname, or longname.
        """
        self._ensure_loaded()
        return self._resolve(identifier)

    def get_units(self, identifier: "int | str") -> str:
        """
        Return the units string for a parameter.

        Parameters
        ----------
        identifier : int or str
            A param ID (int), shortname, or longname.

        Returns
        -------
        str
            The units string, or ``"unknown"`` if not available.
        """
        self._ensure_loaded()
        entry = self._resolve(identifier)
        return entry.get("units", "unknown") or "unknown"

    def get_all_by_shortname(self, shortname: str) -> "list[dict]":
        """Return *all* parameter entries that share *shortname*.

        Most short names map to exactly one param ID, but ~163 short names
        are reused across different GRIB parameter tables or originating
        centres.  This method exposes every candidate so callers can inspect
        the collisions and choose the appropriate one.

        Parameters
        ----------
        shortname:
            ECMWF short name to look up.

        Returns
        -------
        list[dict]
            List of metadata dicts, sorted by ascending param ID.  Each dict
            contains at minimum ``id``, ``shortname``, and ``longname``.

        Raises
        ------
        KeyError
            If *shortname* is not found in the database at all.

        Examples
        --------
        >>> db = ParamDB()
        >>> entries = db.get_all_by_shortname("t")
        >>> [(e["id"], e["longname"]) for e in entries]
        [(130, 'Temperature'), (500014, 'Temperature')]
        """
        self._ensure_loaded()
        if shortname not in self._by_shortname_all:
            raise KeyError(f"Short name {shortname!r} not found in database")
        return sorted(self._by_shortname_all[shortname], key=lambda e: e["id"])

    def shortname_has_collisions(self, shortname: str) -> bool:
        """Return ``True`` if *shortname* maps to more than one param ID.

        Parameters
        ----------
        shortname:
            ECMWF short name to check.

        Raises
        ------
        KeyError
            If *shortname* is not found in the database at all.
        """
        self._ensure_loaded()
        if shortname not in self._by_shortname_all:
            raise KeyError(f"Short name {shortname!r} not found in database")
        return len(self._by_shortname_all[shortname]) > 1


try:
    lib = PatchedLib()
except CFFIModuleLoadFailed as e:
    warnings.warn(
        f"MetKit C library could not be loaded: {e}. "
        "MarsRequest and parse() will not be available. ParamDB is still usable.",
        ImportWarning,
        stacklevel=2,
    )
    lib = None
