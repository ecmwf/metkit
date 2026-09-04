# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from typing import IO

from pymetkit.pymetkit_type import (
    UserInputMapper,
    parse_marsrequests,
    MetKitException,
    MarsRequest,
)


def parse_mars_request(file_or_str: IO | str, strict: bool = False) -> list[MarsRequest]:
    """
    Parse one or more MARS requests from a file-like object or a string.

    Parameters
    ----------
    file_or_str : str | IO
        A string or file-like object containing one or more MARS requests.
    strict : bool
        Whether to raise an error (True) or a warning (False) when a request is
        not compatible with the MARS language definition. When False, the
        incompatible parameters are unset from the request.

    Returns
    -------
    list[MarsRequest]
    """
    text = file_or_str if isinstance(file_or_str, str) else file_or_str.read()
    try:
        requests = parse_marsrequests(text, strict)
    except RuntimeError as error:
        raise MetKitException(str(error)) from error
    return [UserInputMapper.mars_request_from_internal(request) for request in requests]
