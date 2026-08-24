# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# libmetkit.so and dependencies have to be loaded prior to importing
# pymetkit
import findlibs

findlibs.load("metkit")

from pymetkit._internal.pymetkit_internal import (
    MetKitException,
)
from pymetkit_bindings.pymetkit_bindings import (
    MarsRequest as _MarsRequest,
)
from pymetkit_bindings.pymetkit_bindings import (
    init_bindings,
    parse_marsrequest,
    parse_marsrequests,
    version_info,
)

__all__ = [
    "init_bindings",
    "version_info",
    "parse_marsrequest",
    "parse_marsrequests",
    "_MarsRequest",
    "MetKitException",
]
