# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Equality, hashing and string representation of MarsRequest."""

from pymetkit import MarsRequest


def base_request(verb="retrieve", **overrides):
    selection = {
        "class": "od",
        "domain": "g",
        "date": "20230101",
        "param": "130",
        "expver": "0001",
        "step": range(0, 13, 6),
    }
    selection.update(overrides)
    return MarsRequest(verb, selection)


# ---------------------------------------------------------------------------
# Equal requests
# ---------------------------------------------------------------------------


def test_identical_requests_are_equal():
    assert base_request() == base_request()


def test_equality_ignores_value_representation():
    # 20230101 vs "20230101" and 130 vs "130" expand to the same request
    assert base_request() == base_request(date=20230101, param=130)


# ---------------------------------------------------------------------------
# Unequal requests
# ---------------------------------------------------------------------------


def test_different_verb_is_not_equal():
    assert base_request(verb="retrieve") != base_request(verb="compute")


def test_different_values_are_not_equal():
    assert base_request(param="130") != base_request(param="131")


def test_comparison_with_non_request_is_not_equal():
    assert (base_request() == "retrieve") is False
    assert base_request() != "retrieve"


# ---------------------------------------------------------------------------
# Representation
# ---------------------------------------------------------------------------


def test_repr_contains_the_verb():
    assert "retrieve" in repr(base_request())


# ---------------------------------------------------------------------------
# Hashing
# ---------------------------------------------------------------------------


def test_equal_requests_have_same_hash():
    assert hash(base_request()) == hash(base_request())


def test_hash_consistent_with_equality_across_representations():
    # date as int vs string, param as int vs string: different pre-expansion
    # forms that expand to the same request and must therefore hash identically.
    assert hash(base_request()) == hash(base_request(date=20230101, param=130))


def test_unequal_requests_have_different_hash():
    assert hash(base_request(param="130")) != hash(base_request(param="131"))


def test_request_usable_as_dict_key():
    req = base_request()
    d = {req: "value"}
    assert d[req] == "value"


def test_request_usable_in_set():
    req = base_request()
    assert req in {req}
