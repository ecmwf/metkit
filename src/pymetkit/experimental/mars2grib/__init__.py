import findlibs
import warnings

findlibs.load("metkit")

from .mars2grib import Mars2Grib

from ._internal import version_info, __mars2grib_build_version__

_mars2grib_runtime_version = next(
    (version for name, version, _, _ in version_info() if name == "metkit"),
    None,
)
if _mars2grib_runtime_version is not None and _mars2grib_runtime_version != __mars2grib_build_version__:
    warnings.warn(
        f"pyfdb was built against mars2grib {__mars2grib_build_version__} but the loaded "
        f"libmars2grib is version {_mars2grib_runtime_version}. "
        "Behaviour may be unexpected.",
        UserWarning,
        stacklevel=2,
    )


__all__ = ["Mars2Grib"]
