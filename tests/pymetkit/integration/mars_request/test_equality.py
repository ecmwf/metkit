# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Equality and string representation of MarsRequest.

Equality is expansion-backed: both sides are expanded against the MARS language
definition before comparing. These tests therefore require MARS language
definitions to be present in the environment.
"""

from pymetkit import MarsRequest


def base_request(**overrides):
    selection = {
        "class": "od",
        "domain": "g",
        "date": "20230101",
        "param": "130",
        "expver": "0001",
        "step": range(0, 13, 6),
    }
    selection.update(overrides)
    return MarsRequest("retrieve", selection)


# ---------------------------------------------------------------------------
# Equal requests
# ---------------------------------------------------------------------------


def test_identical_requests_are_equal():
    assert base_request() == base_request()


def test_equality_ignores_value_representation():
    # int vs string date and param expand to the same request
    assert base_request() == base_request(date=20230101, param=130)


def test_step_range_equals_explicit_list():
    # range(0, 13, 6) and [0, 6, 12] normalise to the same values before expansion
    assert base_request(step=range(0, 13, 6)) == base_request(step=[0, 6, 12])


# ---------------------------------------------------------------------------
# Unequal requests
# ---------------------------------------------------------------------------


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
