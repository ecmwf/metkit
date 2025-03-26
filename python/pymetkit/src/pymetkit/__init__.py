import cffi
import findlibs

_lib = None

# TODO expose the full functionality

def _get_lib():
    global _lib
    if _lib is None:
        ffi = cffi.FFI()
        ffi.cdef("unsigned int metkit_version_int();")
        loc = findlibs.find("metkit")
        _lib = ffi.dlopen(loc)
    return _lib

def metkit_version_int() -> int:
    return _get_lib().metkit_version_int()
