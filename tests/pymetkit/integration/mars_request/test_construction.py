# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Construction of MarsRequest objects: verbs and selections."""

import pytest

from pymetkit import MarsRequest

# ---------------------------------------------------------------------------
# Verb and selection sources
# ---------------------------------------------------------------------------


def test_verb_only():
    req = MarsRequest("retrieve")
    assert req.verb() == "retrieve"
    assert list(req.keys()) == []


def test_selection_mapping_argument():
    req = MarsRequest("retrieve", {"class": "od", "param": [151, 129]})
    assert req["class"] == "od"
    assert req["param"] == ["151", "129"]


# ---------------------------------------------------------------------------
# Value normalization
# ---------------------------------------------------------------------------


def test_numeric_values_are_stringified():
    req = MarsRequest("retrieve", {"step": [0, 6, 12], "number": 1, "threshold": 1.5})
    assert req["step"] == ["0", "6", "12"]
    assert req["number"] == "1"
    assert req["threshold"] == "1.5"


def test_range_value_is_expanded_to_strings():
    req = MarsRequest("retrieve", {"step": range(0, 13, 6)})
    assert req["step"] == ["0", "6", "12"]


def test_slash_separated_string_is_split():
    req = MarsRequest("retrieve", {"date": "20200101/20200102", "step": "0/to/24/by/6"})
    assert req["date"] == ["20200101", "20200102"]
    assert req["step"] == ["0", "to", "24", "by", "6"]


# ---------------------------------------------------------------------------
# Rejected input
# ---------------------------------------------------------------------------


def test_mapping_value_is_rejected():
    with pytest.raises(ValueError):
        MarsRequest("retrieve", {"bad": {"nested": 1}})


def test_unsupported_value_object_is_rejected():
    with pytest.raises(ValueError):
        MarsRequest("retrieve", {"bad": object()})


def test_non_mapping_selection_is_rejected():
    with pytest.raises(ValueError):
        MarsRequest("retrieve", [("class", "od")])
