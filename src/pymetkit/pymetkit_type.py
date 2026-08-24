# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from collections.abc import Collection, Mapping

InternalMarsSelection = dict[str, list[str]]
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
            f"MarsSelection: unknown type for key '{key}'. Values must be int, float, "
            "str or a collection of those."
        )
