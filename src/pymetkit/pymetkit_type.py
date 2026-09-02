# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from collections.abc import Collection, Mapping

InternalMarsSelection = dict[str, list[str]]
"""
Bindings-layer representation of a MARS selection: ``{key: [str, ...]}``.

All values are lists of strings. This is the form passed to and received from
the ``pymetkit_bindings`` extension module.
"""

MarsSelection = Mapping[str, "str | int | float | Collection[str | int | float]"]
"""
User-facing MARS selection: a mapping from parameter names to values.

Each value may be a scalar (``str``, ``int``, ``float``) or a collection of
those. A ``str`` containing ``/`` is treated as a MARS range expression and
split by :meth:`UserInputMapper.map_selection_to_internal`.
"""


class UserInputMapper:
    """
    Converts between user-supplied :data:`MarsSelection` values and the
    ``dict[str, list[str]]`` form required by the bindings layer.
    """

    @classmethod
    def map_selection_to_internal(cls, selection: MarsSelection) -> InternalMarsSelection:
        """Convert a :data:`MarsSelection` to ``dict[str, list[str]]``.

        Scalars are wrapped in a single-element list, collections are
        stringified element-by-element, and ``/``-separated strings are split.
        """
        result: InternalMarsSelection = {}

        for key, values in selection.items():
            result[key] = cls._normalize_values(key, values)

        return result

    @classmethod
    def map_selection_to_external(cls, selection: InternalMarsSelection) -> MarsSelection:
        """Convert an :data:`InternalMarsSelection` to a user-friendly form.

        Single-element lists are collapsed to a plain ``str``; multi-element
        lists are returned as-is.
        """
        result = {}

        for key, values in selection.items():
            result[key] = cls.map_values_to_external(values)

        return result

    @staticmethod
    def map_values_to_external(values: Collection[str]) -> str | list[str]:
        """Return a scalar for single-element lists, or the list unchanged."""
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
