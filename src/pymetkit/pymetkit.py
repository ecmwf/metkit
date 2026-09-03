# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from collections.abc import Mapping
from typing import Iterator

from pymetkit._internal import MetKitException, _MarsRequest
from pymetkit.pymetkit_type import MarsSelection, UserInputMapper


class MarsRequest:
    """
    A MARS request: a verb and a :data:`MarsSelection` of parameter values.

    The selection is normalised at construction time: scalars are wrapped in a
    single-element list, collections are stringified element-by-element, and
    ``/``-separated strings are split into lists.

    Parameters
    ----------
    verb : str
        The request verb, e.g. ``"retrieve"``.
    selection : MarsSelection, optional
        Initial parameter values.

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
        self._internal = _MarsRequest(verb)
        if selection:
            for param, values in UserInputMapper.map_selection_to_internal(selection).items():
                self._internal.set(param, values)

    # -- Construction / conversion helpers ---------------------------------

    def _to_internal(self) -> _MarsRequest:
        return self._internal

    @classmethod
    def _from_internal(cls, internal: _MarsRequest) -> "MarsRequest":
        request = cls.__new__(cls)
        request._internal = internal
        return request

    # -- Queries -----------------------------------------------------------

    def verb(self) -> str:
        """Return the verb of the request."""
        return self._internal.verb()

    def keys(self) -> Iterator[str]:
        """Return an iterator over the parameter names."""
        return iter(self._internal.params())

    def num_values(self, param: str) -> int:
        """Return the number of values stored for *param*."""
        try:
            return len(self._internal.values(param))
        except RuntimeError:
            raise KeyError(param)

    # -- Operations backed by the MARS language engine --------------------

    def expand(self, inherit: bool = True, strict: bool = False) -> "MarsRequest":
        """
        Return the expanded request.

        .. note::
           When expanding more than one request, prefer
           :func:`~pymetkit.pymetkit_batch.expand`: it performs internal language
           checks once for the whole batch.

        Parameters
        ----------
        inherit : bool
            If True, populate the expanded request with default values.
        strict : bool
            If True, raise an error instead of a warning for invalid values.

        Returns
        -------
        MarsRequest
            The expanded request.

        Raises
        ------
        MetKitException
            If the request is incompatible with the MARS language definition.
        """
        try:
            return MarsRequest._from_internal(self._internal.expand(inherit, strict))
        except RuntimeError as error:
            raise MetKitException(str(error)) from error

    def validate(self) -> None:
        """
        Validate the request against the MARS language definition.

        Expands without inheriting defaults. Raises if any value is invalid.

        Raises
        ------
        MetKitException
            If the request is incompatible with the MARS language definition.
        """
        self.expand(inherit=False, strict=True)

    def merge(self, other: "MarsRequest") -> "MarsRequest":
        """
        Merge *other* into this request and return the result as a new object.

        Neither input is modified. Values from *self* take precedence; values
        from *other* that are not already present are appended. The result is
        validated against the MARS language definition.

        Parameters
        ----------
        other : MarsRequest
            The request to merge with this one.

        Returns
        -------
        MarsRequest
            The merged request.

        Raises
        ------
        ValueError
            If the two requests do not carry the same set of parameters.
        MetKitException
            If the merged result is incompatible with the MARS language definition.
        """
        if set(self.keys()) != set(other.keys()):
            raise ValueError("Cannot merge requests with different parameters.")

        # C++ merge() modifies the receiver in-place; work on a copy.
        copy = _MarsRequest(self._internal)
        try:
            copy.merge(other._internal)
        except RuntimeError as error:
            raise MetKitException(str(error)) from error

        result = MarsRequest._from_internal(copy)
        result.validate()
        return result

    # -- Mapping-like interface -------------------------------------------

    def __iter__(self) -> Iterator[tuple[str, "str | list[str]"]]:
        """Yield ``(name, value)`` pairs. Single-value parameters yield a scalar."""
        for key in self._internal.params():
            yield key, UserInputMapper.map_values_to_external(self._internal.values(key))

    def __getitem__(self, param: str) -> "str | list[str]":
        """Return the value(s) for *param*. Single-value parameters return a scalar."""
        try:
            return UserInputMapper.map_values_to_external(self._internal.values(param))
        except RuntimeError:
            raise KeyError(param)

    def __setitem__(self, param: str, values) -> None:
        """Set *param* to *values*. Accepts the same value forms as the constructor."""
        self._internal.set(param, UserInputMapper._normalize_values(param, values))

    def __contains__(self, param: str) -> bool:
        """Return ``True`` if *param* is present in the request."""
        return self._internal.has(param)

    def __eq__(self, other: object) -> bool:
        """
        Return ``True`` if both requests are equivalent after expansion.

        MARS language aliases are resolved: ``"od"`` and ``"operations"`` compare
        equal. Raises :exc:`MetKitException` if either request cannot be expanded.
        """
        if not isinstance(other, MarsRequest):
            return NotImplemented
        try:
            left = self._internal.expand(True, False)
            right = other._internal.expand(True, False)
        except RuntimeError as error:
            raise MetKitException(str(error)) from error
        return left.md5() == right.md5()

    def __repr__(self) -> str:
        """Return the MARS request string."""
        return repr(self._internal)
