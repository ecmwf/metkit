# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from collections.abc import Collection, Iterator, Mapping

from ._internal import (
    MetKitException as MetKitException,
)
from ._internal import (
    _MarsRequest,
)
from ._internal import (
    parse_marsrequests as parse_marsrequests,
)

InternalMarsSelection = Mapping[str, list[str]]
"""
Internal representation of a MARS selection.

A key-value map, mapping MARS keys to a list of string values. This is the form
handed to the ``pymetkit_bindings`` layer.
"""

MarsSelection = Mapping[str, "str | int | float | Collection[str | int | float]"]
"""
Selection part of a MARS request: a mapping from MARS keys to user-supplied values.

Values may be a scalar (``str``, ``int``, ``float``) or a collection of those.
A ``str`` containing ``/`` is treated as a MARS range/list expression and split
on ``/`` by :meth:`UserInputMapper.map_selection_to_internal`.
"""


class UserInputMapper:
    """
    Normalises user-supplied MARS selections to and from the internal
    ``dict[str, list[str]]`` representation used by the bindings layer.

    - :meth:`map_selection_to_internal` converts a user-facing
      :data:`MarsSelection` (scalars, collections, range expressions) to an
      :data:`InternalMarsSelection`.
    - :meth:`map_selection_to_external` converts an :data:`InternalMarsSelection`
      back to a user-friendly form, collapsing single-element lists to scalars.
    """

    @classmethod
    def map_selection_to_internal(cls, selection: MarsSelection) -> InternalMarsSelection:
        """Normalise a user-supplied selection to ``dict[str, list[str]]``.

        Each value is converted to a list of strings: scalars are wrapped in a
        one-element list, collections are stringified element-by-element, and
        ``str`` values containing ``/`` are split on ``/``.
        """
        result: InternalMarsSelection = {}

        for key, values in selection.items():
            result[key] = cls._normalize_values(key, values)

        return result

    @classmethod
    def map_selection_to_external(cls, selection: InternalMarsSelection) -> MarsSelection:
        """Convert an internal selection back to a user-friendly form.

        Each ``list[str]`` value is collapsed to a plain ``str`` if it contains
        a single element, or left as a ``list[str]`` if it contains multiple.
        """
        result = {}

        for key, values in selection.items():
            result[key] = cls.map_values_to_external(values)

        return result

    @classmethod
    def mars_request_to_internal(cls, mars_request: "MarsRequest") -> _MarsRequest:
        internal = _MarsRequest(mars_request._verb)
        for param, values in mars_request.selection.items():
            internal.set(param, list(values))
        return internal

    @classmethod
    def mars_request_from_internal(cls, internal: _MarsRequest) -> "MarsRequest":
        verb = internal.verb()
        parameters = {}
        for param in internal.params():
            parameters[param] = internal.values(param)
        return MarsRequest(verb, parameters)

    @staticmethod
    def map_values_to_external(values: Collection[str]) -> str | list[str]:
        """Collapse a single-element list to a scalar; return multi-element lists as-is."""
        values = list(values)
        if len(values) == 1:
            return values[0]
        return values

    @staticmethod
    def _normalize_values(key: str, values) -> list[str]:
        if not isinstance(values, (int, float, str, Collection)) or isinstance(values, Mapping):
            raise ValueError(
                f"MarsSelection: the value for key '{key}' is not valid. Values must be "
                "int, float, str or a collection of those."
            )

        # Values is a collection but not a single string
        if isinstance(values, Collection) and not isinstance(values, str):
            return [str(value) if isinstance(value, (float, int)) else value for value in values]
        # Single numeric value
        if isinstance(values, (int, float)):
            return [str(values)]
        # Single string; '/'-separated range/list expressions are split
        if isinstance(values, str):
            if "/" in values:
                return values.split("/")
            return [values]

        raise ValueError(
            f"MarsSelection: unknown type for key '{key}'. Values must be int, float, str or a collection of those."
        )


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
        if selection is not None and not isinstance(selection, Mapping):
            raise ValueError(f"MarsRequest: expected a mapping, got {type(selection).__name__}.")
        combined: MarsSelection = selection if selection is not None else {}

        self.selection: InternalMarsSelection = UserInputMapper.map_selection_to_internal(combined)
        self._verb = verb
        self._internal = _MarsRequest(verb, UserInputMapper.map_selection_to_internal(combined))

    def verb(self) -> str:
        """Return the request verb."""
        return self._verb

    def keys(self) -> Iterator[str]:
        """Return an iterator over the parameter names in the request."""
        return iter(self.selection.keys())

    def num_values(self, param: str) -> int:
        """Return the number of values for a parameter."""
        return len(self.selection[param])

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
            expanded = UserInputMapper.mars_request_to_internal(self).expand(inherit, strict)
        except RuntimeError as error:
            raise MetKitException(str(error)) from error
        return UserInputMapper.mars_request_from_internal(expanded)

    def split(self, keys: list[str]) -> list["MarsRequest"]:
        resulting_requests = self._internal.split(keys)
        return [UserInputMapper.mars_request_from_internal(req) for req in resulting_requests]

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
        internal = UserInputMapper.mars_request_to_internal(self)
        try:
            internal.merge(UserInputMapper.mars_request_to_internal(other))
        except RuntimeError as error:
            raise MetKitException(str(error)) from error
        result = UserInputMapper.mars_request_from_internal(internal)
        result.validate()
        return result

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
        return repr(self._internal)
