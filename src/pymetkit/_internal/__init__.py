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
    expand_marsrequests,
    init_bindings,
    parse_marsrequest,
    parse_marsrequests,
    version_info,
    __metkit_build_version__ as _metkit_build_version,
)


def _check_metkit_version_compatibility(build_version, runtime_info):
    matches = [version for name, version, _, _ in runtime_info if name == "metkit"]
    runtime_version = matches[0] if matches else None
    if runtime_version is None:
        raise RuntimeError(
            "pymetkit could not determine the version of the loaded libmetkit. "
            "The library may not have loaded correctly. "
            "Run 'python -m pymetkit --print-home-deps' to inspect the dependency setup."
        )
    if runtime_version != build_version:
        warnings.warn(
            f"pymetkit was built against metkit {build_version} but the loaded "
            f"libmetkit is version {runtime_version}. "
            "Behaviour may be unexpected. "
            "Run 'python -m pymetkit --print-home-deps' to inspect which libraries "
            "were picked up, or consult the pymetkit documentation.",
            UserWarning,
            stacklevel=2,
        )


_check_metkit_version_compatibility(_metkit_build_version, version_info())

__all__ = [
    "init_bindings",
    "version_info",
    "parse_marsrequest",
    "parse_marsrequests",
    "expand_marsrequests",
    "_MarsRequest",
    "MetKitException",
]
