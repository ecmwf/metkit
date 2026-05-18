import json
import os
import importlib.resources
from cffi import FFI
import findlibs
from datetime import datetime, timedelta, timezone
from typing import IO, Iterator
import warnings
from pathlib import Path
import yaml
from ._version import __version__

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


class MarsRequest:
    def __init__(self, verb: str | None = None, **kwargs):
        """
        Create MetKit MarsRequest object. Parameters and values in
        the request can be specified through kwargs, noting that
        reserved words in Python must be suffixed with "_" e.g. "class_"
        """
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
        self.__lib = ffi.dlopen(libName)

        # All of the executable members of the CFFI-loaded library are functions in the MetKit
        # C API. These should be wrapped with the correct error handling. Otherwise forward
        # these on directly.

        for f in dir(self.__lib):
            try:
                attr = getattr(self.__lib, f)
                setattr(
                    self, f, self.__check_error(attr, f) if callable(attr) else attr
                )
            except Exception as e:
                print(e)
                print("Error retrieving attribute", f, "from library")

        # Initialise the library, and set it up for python-appropriate behaviour

        self.metkit_initialise()

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
    """

    _API_URL = "https://codes.ecmwf.int/parameter-database/api/v1/param/"

    #: File name written inside the platform cache directory.
    _CACHE_FILENAME = "paramdb_online_cache.json"

    #: Default time-to-live for the online cache.
    _DEFAULT_CACHE_TTL = timedelta(hours=1)

    #: Default HTTP request timeout in seconds for online API calls.
    _REQUEST_TIMEOUT = 30

    def __init__(
        self,
        mode: str = "offline",
        cache_ttl: "timedelta | None" = None,
        cache_path: "Path | str | None" = None,
        yaml_path: "Path | str | None" = None,
    ):
        """
        Initialise the parameter database.

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

        self._by_id: dict[int, dict] = {}
        self._by_shortname: dict[str, dict] = {}
        self._by_shortname_all: dict[str, list[dict]] = {}
        self._by_longname: dict[str, dict] = {}

        if mode == "online":
            effective_ttl = self._DEFAULT_CACHE_TTL if cache_ttl is None else cache_ttl
            if not isinstance(effective_ttl, timedelta):
                raise TypeError(
                    f"cache_ttl must be a datetime.timedelta, got {type(effective_ttl).__name__!r}"
                )
            self._load_online(cache_ttl=effective_ttl, cache_path=cache_path)
        else:
            self._load_offline(yaml_path=yaml_path)

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
        center: "int | None" = None,
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
        center:
            WMO originating centre number (e.g. ``98`` for ECMWF, ``7`` for
            NCEP).  Only relevant for param IDs ≥ 1 000 000.  When provided,
            only candidates from that centre are considered.

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

        if table is None and center is None:
            # No context — return the default (lowest id / first-write-wins).
            return self._by_shortname[shortname]

        filtered = candidates
        if table is not None:
            filtered = [e for e in filtered if self._table_from_id(e["id"]) == table]
        if center is not None:
            filtered = [e for e in filtered if self._center_from_id(e["id"]) == center]

        if not filtered:
            ctx_parts = []
            if table is not None:
                ctx_parts.append(f"table={table}")
            if center is not None:
                ctx_parts.append(f"center={center}")
            raise KeyError(
                f"Short name {shortname!r} not found for context "
                f"{', '.join(ctx_parts)}"
            )

        # Among the filtered candidates return the one with the lowest id
        # (most canonical).
        return min(filtered, key=lambda e: e["id"])

    @staticmethod
    def _normalise(raw: dict) -> dict:
        """Return a normalised parameter dict with canonical key names."""
        entry = dict(raw)

        # Normalise shortname
        for key in ("shortName", "short_name"):
            if key in entry:
                entry["shortname"] = entry.pop(key)
                break

        # Normalise longname (online API may return 'name', 'longName', or 'long_name')
        if "longname" not in entry:
            for key in ("longName", "long_name", "name"):
                if key in entry:
                    entry["longname"] = entry.pop(key)
                    break

        # Ensure integer id
        if "id" in entry:
            entry["id"] = int(entry["id"])

        return entry

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

    def _load_online(
        self, cache_ttl: timedelta, cache_path: "Path | str | None"
    ) -> None:
        if _requests is None:
            raise ImportError(
                "The 'requests' package is required for online mode. "
                "Install it with: pip install requests"
            )

        # Try the cache first (unless TTL is zero)
        if cache_ttl > timedelta(0):
            cached = self._read_cache(cache_path, cache_ttl)
            if cached is not None:
                for raw in cached:
                    self._index(self._normalise(raw))
                return

        # Fetch from the API
        response = _requests.get(self._API_URL, timeout=self._REQUEST_TIMEOUT)
        response.raise_for_status()
        params = response.json()

        # Persist to cache (best-effort; errors are silently ignored)
        if cache_ttl > timedelta(0):
            self._write_cache(params, cache_path)

        for raw in params:
            self._index(self._normalise(raw))

    def _load_offline(self, yaml_path: "Path | str | None" = None) -> None:
        if yaml_path is not None:
            resolved = Path(yaml_path)
            if not resolved.exists():
                raise FileNotFoundError(f"Custom YAML file not found: {resolved}")
        else:
            resolved = self._find_offline_yaml()
        with resolved.open("r") as fh:
            params = yaml.safe_load(fh)
        for raw in params:
            self._index(self._normalise(raw))

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
        center: "int | None" = None,
    ) -> str:
        """Return the long name for *shortname*.

        Parameters
        ----------
        shortname:
            ECMWF short name (e.g. ``"t"``, ``"tp"``).
        table:
            Optional GRIB parameter table number to disambiguate collisions
            (e.g. ``128`` for classic ECMWF, ``140`` for ocean waves).
        center:
            Optional WMO originating centre number (e.g. ``98`` for ECMWF,
            ``7`` for NCEP).  Only relevant for param IDs ≥ 1 000 000.
        """
        return self._resolve_shortname_with_context(shortname, table, center)["longname"]

    def longname_to_shortname(self, longname: str) -> str:
        if longname not in self._by_longname:
            raise KeyError(f"Long name {longname!r} not found in database")
        return self._by_longname[longname]["shortname"]

    def shortname_to_param_id(
        self,
        shortname: str,
        table: "int | None" = None,
        center: "int | None" = None,
    ) -> int:
        """Return the param ID for *shortname*.

        Parameters
        ----------
        shortname:
            ECMWF short name (e.g. ``"t"``, ``"tp"``).
        table:
            Optional GRIB parameter table number to disambiguate collisions
            (e.g. ``128`` for classic ECMWF, ``140`` for ocean waves).
        center:
            Optional WMO originating centre number (e.g. ``98`` for ECMWF,
            ``7`` for NCEP).  Only relevant for param IDs ≥ 1 000 000.
        """
        return int(
            self._resolve_shortname_with_context(shortname, table, center)["id"]
        )

    def param_id_to_shortname(self, param_id: int) -> str:
        if param_id not in self._by_id:
            raise KeyError(f"Parameter id {param_id!r} not found in database")
        return self._by_id[param_id]["shortname"]

    def longname_to_param_id(self, longname: str) -> int:
        if longname not in self._by_longname:
            raise KeyError(f"Long name {longname!r} not found in database")
        return int(self._by_longname[longname]["id"])

    def param_id_to_longname(self, param_id: int) -> str:
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
        if shortname not in self._by_shortname_all:
            raise KeyError(f"Short name {shortname!r} not found in database")
        return len(self._by_shortname_all[shortname]) > 1


try:
    lib = PatchedLib()
except CFFIModuleLoadFailed as e:
    raise ImportError() from e
