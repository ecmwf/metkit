# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from pymetkit_bindings import pymetkit_bindings as pymetkit_internal

# Initial setup of binding via eckit main
pymetkit_internal.init_bindings()


class MetKitException(RuntimeError):
    """Raised when the MetKit library throws an exception."""

    pass
