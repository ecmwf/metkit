# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# libmetkit.so and dependencies have to be loaded prior to importing
# pymetkit
import findlibs
import warnings

findlibs.load("metkit")

from pymetkit._internal.pymetkit_internal import (
    MetKitException,
)
from pymetkit_bindings.pymetkit_bindings import (
    MarsRequest as _MarsRequest,
)
from pymetkit_bindings.pymetkit_bindings import (
    init_bindings,
    __pymetkit_build_version__ as _pymetkit_build_version,
    parse_marsrequest,
    parse_marsrequests,
    version_info,
)

_pymetkit_runtime_version = next(
    (version for name, version, _, _ in version_info() if name == "metkit"),
    None,
)
if _pymetkit_runtime_version is not None and _pymetkit_runtime_version != _pymetkit_build_version:
    warnings.warn(
        f"pyfdb was built against mars2grib {_pymetkit_build_version} but the loaded "
        f"libmars2grib is version {_pymetkit_runtime_version}. "
        "Behaviour may be unexpected.",
        UserWarning,
        stacklevel=2,
    )


__all__ = [
    "init_bindings",
    "version_info",
    "parse_marsrequest",
    "parse_marsrequests",
    "_MarsRequest",
    "MetKitException",
]
