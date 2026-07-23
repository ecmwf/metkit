# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Expansion and validation against the MARS language definition."""

from datetime import datetime, timedelta

import pytest

from pymetkit import MarsRequest, MetKitException

yesterday = (datetime.today() - timedelta(days=1)).strftime("%Y%m%d")


def valid_request():
    return MarsRequest(
        "retrieve",
        {
            "class": "od",
            "domain": "g",
            "date": "-1",
            "expver": "0001",
            "step": range(0, 13, 6),
        },
    )


# ---------------------------------------------------------------------------
# Expansion
# ---------------------------------------------------------------------------


def test_expand_preserves_verb():
    assert valid_request().expand().verb() == "retrieve"


def test_expand_normalizes_relative_date():
    assert valid_request().expand()["date"] == yesterday


def expansion_of_short_names():
    request = valid_request()
    request_different = valid_request()
    request_different["class"] = "operational"
    request_different["domain"] = "global"
    assert request == request_different


def test_expand_inherits_default_values():
    expanded = valid_request().expand()
    assert "param" in expanded


def test_expand_without_inherit_adds_no_defaults():
    request = valid_request()
    inherited = request.expand(inherit=True)
    minimal = request.expand(inherit=False)
    assert minimal.verb() == "retrieve"
    assert set(minimal.keys()) <= set(inherited.keys())


def test_expand_returns_a_new_object_and_leaves_original_untouched():
    request = valid_request()
    keys_before = set(request.keys())
    expanded = request.expand()
    assert expanded is not request
    assert set(request.keys()) == keys_before


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------


def test_validate_accepts_a_valid_request():
    valid_request().validate()  # must not raise


def test_validate_rejects_an_invalid_value():
    request = MarsRequest(
        "retrieve",
        {
            "class": "invalid",
            "domain": "g",
            "date": "-1",
            "expver": "0001",
            "levtype": "sfc",
            "step": range(0, 13, 6),
        },
    )
    with pytest.raises(MetKitException):
        request.validate()
