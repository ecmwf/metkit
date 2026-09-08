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
"""Bindings-layer selection: ``{key: [str, ...]}``. Passed to/from the C++ extension."""

MarsSelection = Mapping[str, "str | int | float | Collection[str | int | float]"]
"""User-facing selection. Values may be scalars, collections, or ``/``-separated strings."""


class UserInputMapper:
    """Converts between user :data:`MarsSelection` values and ``dict[str, list[str]]``."""

    @classmethod
    def map_selection_to_internal(cls, selection: MarsSelection) -> InternalMarsSelection:
        """Normalise every value in *selection* to a ``list[str]``."""
        return {key: cls._normalize_values(key, values) for key, values in selection.items()}

    @classmethod
    def map_selection_to_external(cls, selection: InternalMarsSelection) -> MarsSelection:
        """Collapse single-element lists to plain strings."""
        return {key: cls.map_values_to_external(values) for key, values in selection.items()}

    @staticmethod
    def map_values_to_external(values: Collection[str]) -> "str | list[str]":
        """Return a scalar for a single-element list, else the list."""
        values = list(values)
        if len(values) == 1:
            return values[0]
        return values

    @staticmethod
    def _normalize_values(key: str, values) -> list[str]:
        if not isinstance(values, (int, float, str, Collection)) or isinstance(values, Mapping):
            raise ValueError(
                f"MarsSelection: value for '{key}' must be int, float, str or a collection of those."
            )

        # Collection (non-string): stringify each element
        if isinstance(values, Collection) and not isinstance(values, str):
            return [str(v) if isinstance(v, (float, int)) else v for v in values]
        # Numeric scalar
        if isinstance(values, (int, float)):
            return [str(values)]
        # String: split on '/'
        if "/" in values:
            return values.split("/")
        return [values]


class MarsRequest:
    """A MARS request: a verb and a :data:`MarsSelection`.

    Values are normalised on construction (scalars wrapped, numbers stringified,
    ``/``-separated strings split). The C++ ``_MarsRequest`` is the sole backing store.

    Parameters
    ----------
    verb:
        Request verb, e.g. ``"retrieve"``.
    selection:
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
    """

    def __init__(self, verb: str, selection: MarsSelection | None = None, /):
        if selection is not None and not isinstance(selection, Mapping):
            raise ValueError(f"MarsRequest: expected a mapping, got {type(selection).__name__}.")
        self._internal = _MarsRequest(verb)
        if selection:
            for param, values in UserInputMapper.map_selection_to_internal(selection).items():
                self._internal.set(param, values)

    # -- Construction helpers ----------------------------------------------

    def _to_internal(self) -> _MarsRequest:
        return self._internal

    @classmethod
    def _from_internal(cls, internal: _MarsRequest) -> "MarsRequest":
        request = cls.__new__(cls)
        request._internal = internal
        return request

    # -- Queries -----------------------------------------------------------

    def verb(self) -> str:
        """Return the verb."""
        return self._internal.verb()

    def keys(self) -> Iterator[str]:
        """Iterate over parameter names."""
        return iter(self._internal.params())

    def num_values(self, param: str) -> int:
        """Return the number of values for *param*. Raises ``KeyError`` if absent."""
        try:
            return len(self._internal.values(param))
        except RuntimeError:
            raise KeyError(param)

    # -- Language-engine operations ----------------------------------------

    def expand(self, inherit: bool = True, strict: bool = False) -> "MarsRequest":
        """Return the expanded request.

        Prefer :func:`~pymetkit.pymetkit_batch.expand` for multiple requests.

        Parameters
        ----------
        inherit:
            Populate missing keys with MARS defaults.
        strict:
            Raise on invalid values instead of warning.

        Raises
        ------
        MetKitException
            If the request is incompatible with the MARS language definition.
        """
        try:
            return MarsRequest._from_internal(self._internal.expand(inherit, strict))
        except RuntimeError as error:
            raise MetKitException(str(error)) from error

    def split(self, keys: list[str]) -> list["MarsRequest"]:
        """Return one request per value combination across *keys*.

        Structural operation — does not require the MARS language definitions.

        Parameters
        ----------
        keys:
            Parameters to split on.
        """
        return [MarsRequest._from_internal(req) for req in self._internal.split(keys)]

    def validate(self) -> None:
        """Validate against the MARS language definition without inheriting defaults.

        Raises
        ------
        MetKitException
            If any value is invalid.
        """
        self.expand(inherit=False, strict=True)

    def merge(self, other: "MarsRequest") -> "MarsRequest":
        """Merge *other* into this request and return a new object.

        Neither input is modified. ``self``'s values take precedence; missing
        values from *other* are appended. The result is validated.

        Parameters
        ----------
        other:
            Request to merge with.

        Raises
        ------
        ValueError
            If the parameter sets differ.
        MetKitException
            If the merged result is invalid.
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

    # -- Mapping-like interface --------------------------------------------

    def __iter__(self) -> Iterator[tuple[str, "str | list[str]"]]:
        """Yield ``(name, value)`` pairs. Single-value parameters yield a scalar."""
        for key in self._internal.params():
            yield key, UserInputMapper.map_values_to_external(self._internal.values(key))

    def __getitem__(self, param: str) -> "str | list[str]":
        """Return the value(s) for *param*. Raises ``KeyError`` if absent."""
        try:
            return UserInputMapper.map_values_to_external(self._internal.values(param))
        except RuntimeError:
            raise KeyError(param)

    def __setitem__(self, param: str, values) -> None:
        """Set *param*. Accepts the same value forms as the constructor."""
        self._internal.set(param, UserInputMapper._normalize_values(param, values))

    def __contains__(self, param: str) -> bool:
        return self._internal.has(param)

    def __eq__(self, other: object) -> bool:
        """Expand both sides and compare. Resolves MARS aliases."""
        if not isinstance(other, MarsRequest):
            return NotImplemented
        try:
            left = self._internal.expand(True, False)
            right = other._internal.expand(True, False)
        except RuntimeError as error:
            raise MetKitException(str(error)) from error
        return left.md5() == right.md5()

    def __repr__(self) -> str:
        return repr(self._to_internal())
