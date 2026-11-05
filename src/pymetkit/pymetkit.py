# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from typing import IO

from pymetkit._internal import MetKitException, parse_marsrequests
from pymetkit.pymetkit_type import MarsRequest


def parse_mars_request(file_or_str: IO | str, strict: bool = False) -> list[MarsRequest]:
    """Parse MARS requests from a string or file-like object.

    Parameters
    ----------
    file_or_str:
        Source text or file.
    strict:
        Raise on incompatible parameters instead of unsetting them.
    """
    text = file_or_str if isinstance(file_or_str, str) else file_or_str.read()
    try:
        requests = parse_marsrequests(text, strict)
    except RuntimeError as error:
        raise MetKitException(str(error)) from error
    return [MarsRequest._from_internal(request) for request in requests]
