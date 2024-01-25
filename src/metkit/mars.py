import os
from cffi import FFI
from pkg_resources import parse_version
import findlibs
from typing import IO

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


class ParameterList:
    def __init__(self, request: FFI.CData):
        cparams = ffi.new("metkit_paramiterator_t **")
        lib.metkit_request_get_params(request, cparams)
        self._cdata = ffi.gc(cparams[0], lib.metkit_free_paramiterator)

    def __iter__(self):
        return self

    def __next__(self) -> FFI.CData:
        if lib.metkit_paramiterator_next(self._cdata) == lib.METKIT_ITERATION_COMPLETE:
            raise StopIteration
        cvalue = ffi.new("const char **")
        lib.metkit_paramiterator_param(self._cdata, cvalue)
        return ffi_decode(cvalue[0])


class ValueList:
    def __init__(self, request: FFI.CData, param: str):
        cvalues = ffi.new("metkit_valueiterator_t **")
        lib.metkit_request_get_values(request, ffi_encode(param), cvalues)
        self._cdata = ffi.gc(cvalues[0], lib.metkit_free_valueiterator)

    def __iter__(self):
        return self

    def __next__(self) -> FFI.CData:
        if lib.metkit_valueiterator_next(self._cdata) == lib.METKIT_ITERATION_COMPLETE:
            raise StopIteration
        cvalue = ffi.new("const char **")
        lib.metkit_valueiterator_value(self._cdata, cvalue)
        return ffi_decode(cvalue[0])


class Request(dict):
    def __init__(self, verb: str):
        self.verb = verb

    def set_values(self, param: str, values: list[str]):
        assert param not in self
        self[param] = values

    @staticmethod
    def _from_metkit_request(crequest: FFI.CData) -> "Request":
        cverb = ffi.new("const char **")
        lib.metkit_request_get_verb(crequest, cverb)
        req = Request(ffi_decode(cverb[0]))

        for param in ParameterList(crequest):
            req.set_values(param, [val for val in ValueList(crequest, param)])
        return req

    def __str__(self) -> str:
        return f"verb: {self.verb}, request: {super().__str__()}"


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
        lib.metkit_parse_mars(crequest_iter, ffi_encode(file_or_str))
    else:
        lib.metkit_parse_mars(crequest_iter, ffi_encode(file_or_str.read()))
    request_iter = ffi.gc(crequest_iter[0], lib.metkit_free_requestiterator)

    requests = []
    while lib.metkit_requestiterator_next(request_iter) == lib.METKIT_SUCCESS:
        crequest = ffi.new("metkit_request_t **")
        lib.metkit_requestiterator_request(request_iter, crequest)
        request = ffi.gc(crequest[0], lib.metkit_free_request)
        requests.append(Request._from_metkit_request(request))

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
