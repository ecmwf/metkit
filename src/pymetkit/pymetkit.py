# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from collections.abc import Mapping
from typing import IO, Iterator

from pymetkit._internal import (
    MetKitException,
    _MarsRequest,
    parse_marsrequests,
)
from pymetkit.pymetkit_type import InternalMarsSelection, MarsSelection, UserInputMapper


class MarsRequest:
    """
    A MARS request: a verb (e.g. ``retrieve``) together with a
    :data:`MarsSelection` describing the parameters and their values.

    Parameters
    ----------
    verb : str
        The request verb, e.g. ``retrieve``.
    selection : MarsSelection, optional
        Initial parameter values. Scalars are wrapped in a singleton list,
        collections are stringified, and ``/``-separated strings are split.

    Examples
    --------
    >>> request = MarsRequest("retrieve", {"class": "od", "date": "20200101/20200102", "param": [151, 129]})
    >>> request.verb()
    'retrieve'
    >>> request["class"]
    'od'
    >>> request["date"]
    ['20200101', '20200102']
    >>> request["param"]
    ['151', '129']

    Iterating yields ``(name, value)`` pairs:

    >>> for key, value in request:
    ...     print(key, value)
    class od
    date ['20200101', '20200102']
    param ['151', '129']
    """

    def __init__(self, verb: str, selection: MarsSelection | None = None, /):
        self._verb = verb
        if selection is not None and not isinstance(selection, Mapping):
            raise ValueError(f"MarsRequest: expected a mapping, got {type(selection).__name__}.")
        combined: MarsSelection = selection if selection is not None else {}
        self.selection: InternalMarsSelection = UserInputMapper.map_selection_to_internal(combined)

    # -- Construction / conversion helpers ---------------------------------

    def _to_internal(self) -> _MarsRequest:
        internal = _MarsRequest(self._verb)
        for param, values in self.selection.items():
            internal.set(param, list(values))
        return internal

    @classmethod
    def _from_internal(cls, internal: _MarsRequest) -> "MarsRequest":
        request = cls(internal.verb())
        for param in internal.params():
            request.selection[param] = internal.values(param)
        return request

    # -- Queries -----------------------------------------------------------

    def verb(self) -> str:
        """Return the request verb."""
        return self._verb

    def keys(self) -> Iterator[str]:
        """Return an iterator over the parameter names in the request."""
        return iter(self.selection.keys())

    def num_values(self, param: str) -> int:
        """Return the number of values for a parameter."""
        return len(self.selection[param])

    # -- Operations backed by the MARS language engine --------------------

    def expand(self, inherit: bool = True, strict: bool = False) -> "MarsRequest":
        """
        Return the expanded request.

        Parameters
        ----------
        inherit : bool
            If True, populate the expanded request with default values.
        strict : bool
            If True, raise an error instead of a warning for invalid values.

        Returns
        -------
        MarsRequest
            The request resulting from expansion.
        """
        try:
            expanded = self._to_internal().expand(inherit, strict)
        except RuntimeError as error:
            raise MetKitException(str(error)) from error
        return MarsRequest._from_internal(expanded)

    def validate(self) -> None:
        """
        Check that the request is valid against the MARS language definition.
        Does not inherit missing parameters.

        Raises
        ------
        MetKitException
            If the request is incompatible with the MARS language definition.
        """
        self.expand(inherit=False, strict=True)

    def merge(self, other: "MarsRequest") -> "MarsRequest":
        """
        Merge the values of another request into this one and return the result
        as a new request. Does not modify either input. Both requests must
        contain the same parameters and the result must be compatible with the
        MARS language definition.

        Parameters
        ----------
        other : MarsRequest
            The request to merge with self.

        Returns
        -------
        MarsRequest
            The result of the merge.

        Raises
        ------
        ValueError
            If the parameters in the two requests do not match.
        MetKitException
            If the resulting request is not compatible with the MARS language definition.
        """
        if set(self.keys()) != set(other.keys()):
            raise ValueError("Cannot merge requests with different parameters.")
        internal = self._to_internal()
        try:
            internal.merge(other._to_internal())
        except RuntimeError as error:
            raise MetKitException(str(error)) from error
        result = MarsRequest._from_internal(internal)
        result.validate()
        return result

    # -- Mapping-like interface -------------------------------------------

    def __iter__(self) -> Iterator[tuple[str, str | list[str]]]:
        for key in self.selection:
            yield key, UserInputMapper.map_values_to_external(self.selection[key])

    def __getitem__(self, param: str) -> str | list[str]:
        return UserInputMapper.map_values_to_external(self.selection[param])

    def __setitem__(self, param: str, values) -> None:
        self.selection[param] = UserInputMapper._normalize_values(param, values)

    def __contains__(self, param: str) -> bool:
        return param in self.selection

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, MarsRequest):
            return NotImplemented
        if self.verb() != other.verb():
            return False
        return dict(self.expand()) == dict(other.expand())

    def __hash__(self) -> int:
        expanded = self.expand()
        return hash(
            (
                expanded.verb(),
                frozenset((k, tuple(v)) for k, v in expanded.selection.items()),
            )
        )

    def __repr__(self) -> str:
        return repr(self._to_internal())


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
    return [MarsRequest._from_internal(request) for request in requests]
