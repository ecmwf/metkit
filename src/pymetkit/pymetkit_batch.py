# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Bulk operations over :class:`~pymetkit.pymetkit.MarsRequest` objects."""

from pymetkit._internal import MetKitException, expand_marsrequests
from pymetkit.pymetkit import MarsRequest


def expand(
    mars_requests: "list[MarsRequest] | MarsRequest",
    inherit: bool = True,
    strict: bool = False,
) -> "list[MarsRequest] | MarsRequest":
    """
    Expand one or more requests against the MARS language definition.

    Calling :meth:`~pymetkit.pymetkit.MarsRequest.expand` per request rebuilds
    the language definition for its verb every time. Expanding a batch through
    this method shares one expansion across all of them, so the per-verb
    language is built once.

    Parameters
    ----------
    mars_requests : list[MarsRequest] | MarsRequest
        The request, or requests, to expand.
    inherit : bool
        If True, populate the expanded requests with default values.
    strict : bool
        If True, raise an error instead of a warning for invalid values.

    Returns
    -------
    list[MarsRequest] | MarsRequest
        The expanded requests, matching the shape of the input: a single
        request in returns a single request out.

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
