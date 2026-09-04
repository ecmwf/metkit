# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Merging two MarsRequest objects.

metkit's merge takes the union of the values of matching parameters, keeping the
values of ``self`` first and appending only those values of ``other`` that are
not already present (order-preserving, deduplicated). The two requests must
carry the same parameters and the merged result must validate against the MARS
language definition.
"""

import pytest

from pymetkit import MarsRequest, MetKitException

# Shared parameter set so both operands carry identical keys unless a test
# deliberately diverges.
BASE = {"class": "od", "domain": "g", "expver": "0001", "step": range(0, 13, 6)}


def _left():
    return MarsRequest("retrieve", {**BASE, "date": "-1", "levtype": "sfc"})


def _right():
    return MarsRequest("retrieve", {**BASE, "date": "20230101", "levtype": "sfc"})


# ---------------------------------------------------------------------------
# Successful merge
# ---------------------------------------------------------------------------


def test_merge_unions_values_keeping_self_first():
    merged = _left().merge(_right())
    # date differs between the two -> union, self's value first
    assert merged["date"] == ["-1", "20230101"]


def test_merge_deduplicates_identical_values():
    # levtype is "sfc" on both sides -> a single value, not duplicated
    assert _left().merge(_right())["levtype"] == "sfc"


def test_merge_leaves_self_only_values_unchanged():
    merged = _left().merge(_right())
    assert merged["class"] == "od"
    assert merged["step"] == ["0", "6", "12"]


def test_merge_returns_a_distinct_new_request():
    left, right = _left(), _right()
    merged = left.merge(right)
    assert isinstance(merged, MarsRequest)
    assert merged is not left
    assert merged is not right


def test_merge_does_not_mutate_the_operands():
    left, right = _left(), _right()
    left.merge(right)
    # values are untouched on both inputs
    assert left["date"] == "-1"
    assert right["date"] == "20230101"
    assert set(left.keys()) == set(right.keys())


# ---------------------------------------------------------------------------
# Rejected merges
# ---------------------------------------------------------------------------


def test_merge_with_different_parameters_raises_value_error():
    left = _left()
    right = MarsRequest("retrieve", {**BASE, "date": "-1", "levtype": "sfc", "type": "em"})
    with pytest.raises(ValueError):
        left.merge(right)


def test_merge_producing_invalid_request_raises_metkit_exception():
    left = _left()
    right = MarsRequest("retrieve", {**BASE, "date": "-1", "levtype": "pl"})
    with pytest.raises(MetKitException):
        left.merge(right)
