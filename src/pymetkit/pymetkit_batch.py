# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Bulk operations over :class:`~pymetkit.pymetkit.MarsRequest` objects."""

from typing import IO

from pymetkit._internal import MetKitException, expand_marsrequests, parse_marsrequests
from pymetkit.pymetkit import MarsRequest


def parse_mars_request(file_or_str: IO | str, strict: bool = False) -> list[MarsRequest]:
    """
    Parse one or more MARS requests from a string or file-like object.

    Parameters
    ----------
    file_or_str : str | IO
        MARS request text, or an open file containing it.
    strict : bool
        If True, raise on invalid values instead of discarding them.

    Returns
    -------
    list[MarsRequest]
        The parsed requests, in the order they appear in the input.

    Raises
    ------
    MetKitException
        If the input cannot be parsed.
    """
    text = file_or_str if isinstance(file_or_str, str) else file_or_str.read()
    try:
        requests = parse_marsrequests(text, strict)
    except RuntimeError as error:
        raise MetKitException(str(error)) from error
    return [MarsRequest._from_internal(request) for request in requests]


def expand(
    mars_requests: "list[MarsRequest] | MarsRequest",
    inherit: bool = True,
    strict: bool = False,
) -> "list[MarsRequest] | MarsRequest":
    """
    Expand one or more requests against the MARS language definition.

    Internal language checks are performed once for the whole batch rather than
    once per request. Pass a list whenever expanding more than one request.

    A single :class:`MarsRequest` may also be passed directly; the return type
    matches the input shape.

    Parameters
    ----------
    mars_requests : MarsRequest | list[MarsRequest]
        The request or requests to expand.
    inherit : bool
        If True, populate the result with default values for missing parameters.
    strict : bool
        If True, raise on invalid values instead of issuing a warning.

    Returns
    -------
    MarsRequest | list[MarsRequest]
        The expanded request(s). A single input returns a single output.

    Raises
    ------
    MetKitException
        If a request is incompatible with the MARS language definition.

    Examples
    --------
    >>> requests = [
    ...     MarsRequest("retrieve", {"class": "od", "date": "-1", "param": "130"}),
    ...     MarsRequest("retrieve", {"class": "od", "date": "-1", "param": "131"}),
    ... ]
    >>> expanded = expand(requests)
    >>> len(expanded)
    2
    """
    is_single = isinstance(mars_requests, MarsRequest)
    requests = [mars_requests] if is_single else list(mars_requests)

    if not requests:
        return []

    internal = [request._to_internal() for request in requests]

    try:
        expanded = expand_marsrequests(internal, inherit, strict)
    except RuntimeError as error:
        raise MetKitException(str(error)) from error

    result = [MarsRequest._from_internal(request) for request in expanded]

    return result[0] if is_single else result
