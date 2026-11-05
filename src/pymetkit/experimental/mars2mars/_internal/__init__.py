import findlibs
import warnings

findlibs.load("metkit")

from .mars2mars_bindings import init_bindings, Mars2Mars as Mars2Mars, __mars2mars_build_version__, version_info

init_bindings()

_mars2mars_runtime_version = next(
    (version for name, version, _, _ in version_info() if name == "metkit"),
    None,
)
if _mars2mars_runtime_version is not None and _mars2mars_runtime_version != __mars2mars_build_version__:
    warnings.warn(
        f"pyfdb was built against mars2mars {__mars2mars_build_version__} but the loaded "
        f"libmars2grib is version {_mars2mars_runtime_version}. "
        "Behaviour may be unexpected.",
        UserWarning,
        stacklevel=2,
    )


__all__ = ["Mars2Mars"]
