# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from collections.abc import Mapping
from typing import IO, Iterator

from pymetkit._internal import MetKitException, _MarsRequest
from pymetkit.pymetkit_type import InternalMarsSelection, MarsSelection, UserInputMapper


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
        self._verb = verb
        if selection is not None and not isinstance(selection, Mapping):
            raise ValueError(f"MarsRequest: expected a mapping, got {type(selection).__name__}.")
        combined: MarsSelection = selection if selection is not None else {}
        self.selection: InternalMarsSelection = UserInputMapper.map_selection_to_internal(combined)
        self._hash_cache: "int | None" = None

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
        """Return the verb of the request."""
        return self._verb

    def keys(self) -> Iterator[str]:
        """Return an iterator over the parameter names."""
        return iter(self.selection.keys())

    def num_values(self, param: str) -> int:
        """Return the number of values stored for *param*."""
        return len(self.selection[param])

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
            expanded = MarsRequest._from_internal(self._to_internal().expand(inherit, strict))
        except RuntimeError as error:
            raise MetKitException(str(error)) from error
        return expanded

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
        """Yield ``(name, value)`` pairs. Single-value parameters yield a scalar."""
        for key in self.selection:
            yield key, UserInputMapper.map_values_to_external(self.selection[key])

    def __getitem__(self, param: str) -> str | list[str]:
        """Return the value(s) for *param*. Single-value parameters return a scalar."""
        return UserInputMapper.map_values_to_external(self.selection[param])

    def __setitem__(self, param: str, values) -> None:
        """Set *param* to *values*. Accepts the same value forms as the constructor."""
        self._hash_cache = None
        self.selection[param] = UserInputMapper._normalize_values(param, values)

    def __contains__(self, param: str) -> bool:
        """Return ``True`` if *param* is present in the request."""
        return param in self.selection

    def __eq__(self, other: object) -> bool:
        """
        Return ``True`` if both requests have identical verb and parameter values.

        Equality is based on the pre-expansion form of the request. MARS language
        aliases are not resolved: ``"od"`` and ``"operations"`` are distinct values.
        """
        if not isinstance(other, MarsRequest):
            return NotImplemented
        if self._hash_cache is None:
            self._hash_cache = hash(self)
        if other._hash_cache is None:
            other._hash_cache = hash(other)
        return self._hash_cache == other._hash_cache

    def __hash__(self) -> int:
        """
        Return a hash of the pre-expansion form of the request.

        Computed on first access and cached. Mutating the request via
        ``__setitem__`` invalidates the cache.
        """
        if self._hash_cache is None:
            self._hash_cache = hash(self._to_internal().md5())
        return hash(self._hash_cache)

    def __repr__(self) -> str:
        """Return the MARS request string."""
        return repr(self._to_internal())
