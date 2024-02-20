import os
from cffi import FFI
from pkg_resources import parse_version
import findlibs
from typing import IO, Iterator

__metkit_version__ = "1.11.0"

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


class Request:
    """
    Class for creating MetKit MarsRequest
    """

    def __init__(self, verb: str | None = None):
        crequest = ffi.new("metkit_request_t **")
        lib.metkit_new_request(crequest)
        self.__request = ffi.gc(crequest[0], lib.metkit_free_request)
        if verb is not None:
            lib.metkit_request_set_verb(self.__request, ffi_encode(verb))

    @staticmethod
    def from_dict(verb: str, req: dict) -> "Request":
        """
        Create Request from verb and dictionary of parameters
        and values

        Returns
        -------
        New request containing dictionary items
        """
        request = Request(verb)
        for param, values in req.items():
            request[param] = values
        return request

    def ctype(self) -> FFI.CData:
        return self.__request

    def verb(self) -> str:
        cverb = ffi.new("const char **")
        lib.metkit_request_verb(self.__request, cverb)
        return ffi_decode(cverb[0])

    def expand(self, inherit: bool = True, strict: bool = False) -> "Request":
        """
        Return expanded request

        Params
        ------
        inherit: bool, if True, populates expanded request with default values
        strict: bool, if True, raise error instead of warning for invalid values

        Returns
        -------
        Expanded request
        """
        expanded_request = Request()
        lib.metkit_request_expand(
            self.__request, expanded_request.ctype(), inherit, strict
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

    def params(self) -> Iterator[str]:
        """
        Get iterator over parameters in request

        Returns
        -------
        Iterator over parameter names
        """
        cparams = ffi.new("metkit_paramiterator_t **")
        lib.metkit_request_params(self.__request, cparams)
        self._cdata = ffi.gc(cparams[0], lib.metkit_free_paramiterator)

        while lib.metkit_paramiterator_next(self._cdata) == lib.METKIT_SUCCESS:
            cvalue = ffi.new("const char **")
            lib.metkit_paramiterator_param(self._cdata, cvalue)
            yield ffi_decode(cvalue[0])

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
        assert param in self
        cparam = ffi_encode(param)
        count = ffi.new("size_t *", 0)
        lib.metkit_request_count_values(self.__request, cparam, count)
        return count[0]

    def __iter__(self) -> Iterator[tuple[str, list[str]]]:
        for param in self.params():
            yield param, self[param]

    def __getitem__(self, param: str) -> str | list[str]:
        num_values = self.num_values(param)
        values = []
        for index in range(num_values):
            cvalue = ffi.new("const char **")
            lib.metkit_request_value(self.__request, ffi_encode(param), index, cvalue)
            value = ffi_decode(cvalue[0])
            if num_values == 1:
                return value
            values.append(value)
        return values

    def __contains__(self, param: str) -> bool:
        has = ffi.new("bool *", False)
        lib.metkit_request_has_param(self.__request, ffi_encode(param), has)
        return has[0]

    def __setitem__(self, param: str, values: int | str | list[str]):
        if isinstance(values, (str, int)):
            values = [values]
        cvals = []
        for value in values:
            if isinstance(value, int):
                value = str(value)
            cvals.append(ffi.new("const char[]", value.encode("ascii")))
        lib.metkit_request_add(
            self.__request,
            ffi_encode(param),
            ffi.new("const char*[]", cvals),
            len(values),
        )


def parse_mars_request(file_or_str: IO | str) -> list[Request]:
    """
    Function for parsing mars request from file object or string.

    Params
    ------
    file_or_str: string or file-like object, containing mars request

    Returns
    -------
    list of Request
    """
    crequest_iter = ffi.new("metkit_requestiterator_t **")

    if isinstance(file_or_str, str):
        lib.metkit_parse_mars_request(ffi_encode(file_or_str), crequest_iter, False)
    else:
        lib.metkit_parse_mars_request(
            ffi_encode(file_or_str.read()), crequest_iter, False
        )
    request_iter = ffi.gc(crequest_iter[0], lib.metkit_free_requestiterator)

    requests = []
    while lib.metkit_requestiterator_next(request_iter) == lib.METKIT_SUCCESS:
        new_request = Request()
        lib.metkit_requestiterator_request(request_iter, new_request.ctype())
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

        tmp_str = ffi.new("char**")
        self.metkit_version(tmp_str)
        versionstr = ffi.string(tmp_str[0]).decode("utf-8")

        if parse_version(versionstr) < parse_version(__metkit_version__):
            raise CFFIModuleLoadFailed(
                "Version of libmetkit found is too old. {} < {}".format(
                    versionstr, __metkit_version__
                )
            )

    def __read_header(self):
        with open(os.path.join(os.path.dirname(__file__), "metkit_cffi.h"), "r") as f:
            return f.read()

    def __check_error(self, fn, name: str):
        """
        If calls into the MetKit library return errors, ensure that they get
        detected and reported by throwing an appropriate python exception.
        """

        def wrapped_fn(*args, **kwargs):
            retval = fn(*args, **kwargs)
            if retval not in (
                self.__lib.METKIT_SUCCESS,
                self.__lib.METKIT_ITERATION_COMPLETE,
            ):
                err = ffi_decode(self.__lib.metkit_get_error_string(retval))
                msg = "Error in function '{}': {}".format(name, err)
                raise MetKitException(msg)
            return retval

        return wrapped_fn


try:
    lib = PatchedLib()
except CFFIModuleLoadFailed as e:
    raise ImportError() from e
