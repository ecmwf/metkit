# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""The Mapping-like interface of MarsRequest: reading, mutating and iterating."""

import pytest

from pymetkit import MarsRequest


def make_request():
    return MarsRequest("retrieve", {"class": "od", "param": [151, 129], "step": [0]})


# ---------------------------------------------------------------------------
# Reading values
# ---------------------------------------------------------------------------


def test_single_value_returns_scalar():
    req = make_request()
    assert req["class"] == "od"
    assert req["step"] == "0"


def test_multiple_values_return_list():
    assert make_request()["param"] == ["151", "129"]


def test_num_values():
    req = make_request()
    assert req.num_values("param") == 2
    assert req.num_values("class") == 1


def test_dict_view():
    assert dict(make_request()) == {
        "class": "od",
        "param": ["151", "129"],
        "step": "0",
    }


# ---------------------------------------------------------------------------
# Membership and iteration
# ---------------------------------------------------------------------------


def test_contains():
    req = make_request()
    assert "class" in req
    assert "missing" not in req


def test_keys_yields_parameter_names():
    req = make_request()
    assert set(req.keys()) == {"class", "param", "step"}


def test_iter_yields_key_value_pairs():
    req = make_request()
    pairs = list(req)
    assert {k for k, _ in pairs} == {"class", "param", "step"}
    assert dict(pairs) == {"class": "od", "param": ["151", "129"], "step": "0"}


# ---------------------------------------------------------------------------
# Mutating values
# ---------------------------------------------------------------------------


def test_set_scalar_value():
    req = make_request()
    req["expver"] = "0001"
    assert req["expver"] == "0001"
    assert "expver" in req


def test_set_overwrites_existing_value():
    req = make_request()
    req["class"] = "ea"
    assert req["class"] == "ea"


def test_set_list_and_range_values():
    req = make_request()
    req["date"] = ["20200101", "20200102"]
    req["step"] = range(0, 13, 6)
    assert req["date"] == ["20200101", "20200102"]
    assert req["step"] == ["0", "6", "12"]


# ---------------------------------------------------------------------------
# Missing keys
# ---------------------------------------------------------------------------


def test_getitem_missing_key_raises_keyerror():
    with pytest.raises(KeyError):
        make_request()["missing"]


def test_num_values_missing_key_raises_keyerror():
    with pytest.raises(KeyError):
        make_request().num_values("missing")


def test_set_invalid_value_is_rejected():
    with pytest.raises(ValueError):
        make_request()["bad"] = {"nested": 1}
