# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Parsing MARS requests from strings and files."""

from datetime import datetime, timedelta

from pymetkit import MarsRequest, parse_mars_request

yesterday = (datetime.today() - timedelta(days=1)).strftime("%Y%m%d")

MULTIPLE_REQUESTS = """
retrieve,
    class=od,
    domain=g,
    expver=0001,
    levtype=sfc,
    stream=enfo,
    date=-1,
    time=12,
    param=151.128,
    grid=O640,
    step=0/to/24/by/6,
	target=test.grib,
	type=em
retrieve,
    class=od,
    domain=g,
    expver=0001,
    levtype=pl,
    stream=enfo,
    date=-1,
    time=12,
    param=129,
    levelist=500,
    grid=O640,
    step=0/to/24/by/6,
	target=test.grib,
	type=em
"""

SINGLE_REQUEST = "retrieve,class=od,date=-1,time=12,param=129,step=12,target=test.grib"


# ---------------------------------------------------------------------------
# Parsing requests
# ---------------------------------------------------------------------------


def test_parse_single_request_from_string():
    requests = parse_mars_request(SINGLE_REQUEST)
    assert len(requests) == 1
    assert requests[0].verb() == "retrieve"
    assert requests[0].num_values("step") == 1


def test_parse_multiple_requests_from_string():
    requests = parse_mars_request(MULTIPLE_REQUESTS)
    assert len(requests) == 2
    for req in requests:
        assert req.verb() == "retrieve"
        assert req.num_values("step") == 5


def test_parse_from_file_object(tmpdir):
    request_file = f"{tmpdir}/requests"
    with open(request_file, "w") as handle:
        handle.write(MULTIPLE_REQUESTS)
    with open(request_file, "r") as handle:
        requests = parse_mars_request(handle)
    assert len(requests) == 2
    assert "class" in requests[0]
    assert requests[1]["levelist"] == "500"


def test_parse_returns_marsrequest_objects():
    requests = parse_mars_request(MULTIPLE_REQUESTS)
    assert all(isinstance(req, MarsRequest) for req in requests)


def test_parse_expands_relative_date():
    requests = parse_mars_request(MULTIPLE_REQUESTS)
    for req in requests:
        assert req["date"] == yesterday


# ---------------------------------------------------------------------------
# Empty input
# ---------------------------------------------------------------------------


def test_parse_empty_string_returns_no_requests():
    assert parse_mars_request("") == []


def test_parse_empty_file_returns_no_requests(tmpdir):
    request_file = f"{tmpdir}/requests"
    with open(request_file, "w") as handle:
        handle.write("")
    assert parse_mars_request(open(request_file, "r")) == []
