# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Tests for internal value normalisation (pymetkit.pymetkit_type.UserInputMapper)."""

import pytest

from pymetkit.pymetkit_type import UserInputMapper

# ---------------------------------------------------------------------------
# map_selection_to_internal: scalar values
# ---------------------------------------------------------------------------


def test_single_value_internal():
    result = UserInputMapper.map_selection_to_internal({"key-1": "value-1"})
    assert len(result) == 1
    assert "key-1" in result
    assert result["key-1"] == ["value-1"]


def test_int_and_float_become_string_lists():
    result = UserInputMapper.map_selection_to_internal({"number": 1, "threshold": 1.5})
    assert result["number"] == ["1"]
    assert result["threshold"] == ["1.5"]


def test_slash_separated_string_is_split():
    result = UserInputMapper.map_selection_to_internal({"date": "20200101/20200102"})
    assert result["date"] == ["20200101", "20200102"]


def test_string_without_slash_is_kept_whole():
    result = UserInputMapper.map_selection_to_internal({"grid": "O640"})
    assert result["grid"] == ["O640"]


# ---------------------------------------------------------------------------
# map_selection_to_internal: collection values
# ---------------------------------------------------------------------------


def test_collection_values_are_stringified():
    result = UserInputMapper.map_selection_to_internal(
        {
            "key-1": ["value-1", "value-2", "value-3"],
            "key-2": ["value-2"],
            "key-3": ["value-3", 214, 213.54],
            "key-4": [120, 123, 124, 125],
        }
    )
    assert result["key-1"] == ["value-1", "value-2", "value-3"]
    assert result["key-2"] == ["value-2"]
    assert result["key-3"] == ["value-3", "214", "213.54"]
    assert result["key-4"] == ["120", "123", "124", "125"]


def test_range_is_expanded():
    result = UserInputMapper.map_selection_to_internal({"step": range(0, 13, 6)})
    assert result["step"] == ["0", "6", "12"]


# ---------------------------------------------------------------------------
# map_selection_to_internal: mixed numeric types
# ---------------------------------------------------------------------------


def test_to_internal_mixed_numeric_types():
    result = UserInputMapper.map_selection_to_internal(
        {
            "key-1": ["value-2", "value-4"],
            "key-2": ["0.1", 0.2],
            "key-3": [0.1, 0.2],
            "key-4": [1, 2],
        }
    )
    assert result["key-1"] == ["value-2", "value-4"]
    assert result["key-2"] == ["0.1", "0.2"]
    assert result["key-3"] == ["0.1", "0.2"]
    assert result["key-4"] == ["1", "2"]


# ---------------------------------------------------------------------------
# map_selection_to_external
# ---------------------------------------------------------------------------


def test_single_value_external():
    result = UserInputMapper.map_selection_to_external({"key-1": ["value-1"]})
    assert result["key-1"] == "value-1"


def test_multi_value_external():
    result = UserInputMapper.map_selection_to_external(
        {
            "key-1": ["value-1", "value-2", "value-3"],
            "key-2": ["value-2"],
        }
    )
    assert result["key-1"] == ["value-1", "value-2", "value-3"]
    assert result["key-2"] == "value-2"


# ---------------------------------------------------------------------------
# map_values_to_external
# ---------------------------------------------------------------------------


def test_map_values_to_external_collapses_single_value():
    assert UserInputMapper.map_values_to_external(["od"]) == "od"
    assert UserInputMapper.map_values_to_external(["151", "129"]) == ["151", "129"]


# ---------------------------------------------------------------------------
# Key overwrite behavior
# ---------------------------------------------------------------------------


def test_overwrite_key_internal():
    result = UserInputMapper.map_selection_to_internal(
        {
            "key-1": "value-1",
            "key-1": ["value-3", "214", "213.54"],
        }
    )
    assert len(result) == 1
    assert result["key-1"] == ["value-3", "214", "213.54"]


def test_overwrite_key_external():
    result = UserInputMapper.map_selection_to_external(
        {
            "key-1": ["value-1"],
            "key-1": ["value-3", "214", "213.54"],
        }
    )
    assert len(result) == 1
    assert result["key-1"] == ["value-3", "214", "213.54"]


# ---------------------------------------------------------------------------
# Rejected values
# ---------------------------------------------------------------------------


def test_mapping_value_is_rejected():
    with pytest.raises(ValueError):
        UserInputMapper.map_selection_to_internal({"bad": {"nested": 1}})


def test_unsupported_value_object_is_rejected():
    with pytest.raises(ValueError):
        UserInputMapper.map_selection_to_internal({"bad": object()})


# ---------------------------------------------------------------------------
# Pythonic interface
# ---------------------------------------------------------------------------


def test_pythonic_interface():
    result = UserInputMapper.map_selection_to_internal(
        {
            "key-1": ["value-1", "value-2", "value-3"],
            "key-2": 0.1,
            "key-3": list(range(1, 5)),
            "key-4": [0.1, "0.2"],
            "key-5": [1 + 0.5 * x for x in range(2)],
        }
    )
    assert result["key-1"] == ["value-1", "value-2", "value-3"]
    assert result["key-2"] == ["0.1"]
    assert result["key-3"] == ["1", "2", "3", "4"]
    assert result["key-4"] == ["0.1", "0.2"]
    assert result["key-5"] == ["1.0", "1.5"]
