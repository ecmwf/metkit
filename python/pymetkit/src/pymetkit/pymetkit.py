import os
from cffi import FFI
import findlibs
from typing import IO, Iterator
import warnings
from ._version import __version__

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
            lib.metkit_marsrequest_value(self.__request, ffi_encode(param), index, cvalue)
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


def parse_mars_request(file_or_str: IO | str, strict: bool = False) -> list[MarsRequest]:
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


def _expand_param(
    value: str | int | list,
    verb: str,
    context: "MarsRequest | dict | None",
    strict: bool,
) -> list[str]:
    """Resolve the ``param`` key via a full (multi-pass) request expansion.

    ``param`` cannot be expanded in isolation because its result depends on other
    keys. A scoped :class:`MarsRequest` is built from ``context`` (which supplies
    the neighbouring keys such as ``class``/``stream``/``type``/``levtype``), the
    param value is set, the request is expanded, and the resolved ``param`` values
    are returned.
    """
    if isinstance(value, (list, tuple)):
        values = [str(v) for v in value]
    else:
        values = str(value).split("/")

    request = MarsRequest(verb)
    if context is not None:
        if isinstance(context, MarsRequest):
            for key, vals in context:
                request[key] = vals
        else:  # dict-like
            for key, vals in context.items():
                request[key.rstrip("_")] = vals
    request["param"] = values

    expanded = request.expand(inherit=False, strict=strict)
    resolved = expanded["param"]
    return [resolved] if isinstance(resolved, str) else resolved


def expand_key(
    keyword: str,
    value: str | int | list,
    verb: str = "retrieve",
    context: "MarsRequest | dict | None" = None,
    strict: bool = False,
) -> list[str]:
    """Expand and normalise the values of a single MARS key, without building a full request.

    Applies the MARS language rules for ``keyword``: range syntax such as
    ``"1/to/10/by/1"`` is expanded, and per-key normalisation is performed (e.g.
    ``date="-1"`` resolves to a ``yyyymmdd`` date, ``time="6"`` to ``"0600"``).

    Context-sensitive keys (e.g. those whose interpretation depends on other
    keys) consult ``context``. Supply the relevant neighbouring keys there, e.g.
    ``expand_key("levelist", "1/to/10", context={"levtype": "ml"})``.

    Note: this performs single-pass expansion for most keys. ``param`` is a
    special case: its value depends on other keys (``class``/``stream``/``type``/
    ``levtype``) and requires a full multi-pass expansion, so ``expand_key``
    transparently builds a scoped request from ``context``, expands it, and
    returns the resolved ``param`` values, e.g.
    ``expand_key("param", "t", context={"levtype": "sfc"})``.

    Params
    ------
    keyword: name of the MARS key to expand (canonical, alias or unambiguous prefix)
    value: values to expand, as a ``"a/b/c"`` string or a list of tokens
    verb: MARS verb whose language defines the key (defaults to "retrieve")
    context: optional MarsRequest or dict of neighbouring keys for context-sensitive keys
    strict: if True, raise an error on invalid values

    Returns
    -------
    list of expanded/normalised string values

    Examples
    --------
    >>> expand_key("step", "1/to/10/by/1")
    ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10']
    >>> expand_key("param", "t", context={"levtype": "pl"})
    ['130']
    """
    if keyword == "param":
        # 'param' cannot be resolved by single-pass expansion: its value depends
        # on other keys, so it needs the full multi-pass request expansion. We
        # hide that from the caller by building a scoped request from the given
        # context, expanding it, and returning just the resolved param values.
        return _expand_param(value, verb, context, strict)

    if isinstance(value, (list, tuple)):
        value = "/".join(str(v) for v in value)
    else:
        value = str(value)

    context_c = ffi.NULL
    if context is not None:
        if isinstance(context, dict):
            context = MarsRequest(**context)
        context_c = context.ctype()

    it_c = ffi.new("metkit_paramiterator_t **")
    lib.metkit_expand_key(
        ffi_encode(verb),
        ffi_encode(keyword),
        ffi_encode(value),
        context_c,
        strict,
        it_c,
    )
    it = ffi.gc(it_c[0], lib.metkit_paramiterator_delete)

    values = []
    while lib.metkit_paramiterator_next(it) == lib.METKIT_ITERATOR_SUCCESS:
        cvalue = ffi.new("const char **")
        lib.metkit_paramiterator_current(it, cvalue)
        values.append(ffi_decode(cvalue[0]))

    return values


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
            warnings.warn(f"Metkit library version {versionstr} does not match python version {__version__}")

    def __read_header(self):
        with open(os.path.join(os.path.dirname(__file__), "metkit_c.h"), "r") as f:
            return f.read()

    def __check_error(self, fn, name: str):
        """
        If calls into the MetKit library return errors, ensure that they get
        detected and reported by throwing an appropriate python exception.

        Two distinct return-code domains exist in the C API and must not share a
        single success-set, otherwise their overlapping integer values collide
        (e.g. metkit_error_t METKIT_ERROR == metkit_iterator_status_t
        METKIT_ITERATOR_COMPLETE == 1):

        * iterator functions (``*_next`` / ``*_current``) return
          metkit_iterator_status_t, where SUCCESS and COMPLETE are both non-error
          outcomes and only ITERATOR_ERROR indicates failure;
        * every other function returns metkit_error_t, where only METKIT_SUCCESS
          indicates success.
        """

        # Functions that do not return an error/status code at all.
        if name in ("metkit_version", "metkit_git_sha1"):
            return fn

        is_iterator = name.endswith("_next") or name.endswith("_current")

        def wrapped_fn(*args, **kwargs):
            retval = fn(*args, **kwargs)

            if is_iterator:
                if retval == self.__lib.METKIT_ITERATOR_ERROR:
                    err = ffi_decode(self.__lib.metkit_get_error_string(retval))
                    raise MetKitException("Error in function '{}': {}".format(name, err))
                return retval

            if retval != self.__lib.METKIT_SUCCESS:
                err = ffi_decode(self.__lib.metkit_get_error_string(retval))
                raise MetKitException("Error in function '{}': {}".format(name, err))
            return retval

        return wrapped_fn


try:
    lib = PatchedLib()
except CFFIModuleLoadFailed as e:
    raise ImportError() from e
