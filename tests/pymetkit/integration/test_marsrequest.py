# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

from contextlib import nullcontext as does_not_raise
from datetime import datetime, timedelta

import pytest

from pymetkit import MarsRequest, MetKitException, parse_mars_request

request = """
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

yesterday = (datetime.today() - timedelta(days=1)).strftime("%Y%m%d")


def test_parse_file(tmpdir):
    request_file = f"{tmpdir}/requests"
    with open(request_file, "w") as f:
        f.write(request)
    requests = parse_mars_request(open(request_file, "r"))
    assert len(requests) == 2
    for req in requests:
        assert req.verb() == "retrieve"
        assert len(req["step"]) == 5
        assert req["date"] == yesterday
    assert "class" in requests[0]
    assert requests[1]["levelist"] == "500"


def test_parse_string():
    requests = parse_mars_request(request)
    assert len(requests) == 2
    for req in requests:
        assert req.num_values("step") == 5

    requests = parse_mars_request(
        "retrieve,class=od,date=-1,time=12,param=129,step=12,target=test.grib"
    )
    assert len(requests) == 1
    assert requests[0].num_values("step") == 1


def test_empty_request(tmpdir):
    request_file = f"{tmpdir}/requests"
    with open(request_file, "w") as f:
        f.write("")
    requests = parse_mars_request(open(request_file, "r"))
    assert len(requests) == 0


def test_new_request():
    req = MarsRequest("retrieve")
    assert req.verb() == "retrieve"

    req = MarsRequest("request", {"class": "od", "type": "pf", "date": ["20200101", "20200102"]})
    assert req["class"] == "od"
    assert req["type"] == "pf"
    assert req["date"] == ["20200101", "20200102"]


def test_request_from_selection():
    req = MarsRequest("retrieve", {"class": "od", "param": [151, 129]})
    assert req["class"] == "od"
    assert req["param"] == ["151", "129"]


def test_request_from_expand():
    req = MarsRequest(
        "retrieve",
        {
            "class": "od",
            "domain": "g",
            "date": "-1",
            "expver": "0001",
            "step": range(0, 13, 6),
        },
    )
    expanded = req.expand()
    assert expanded.verb() == req.verb()
    assert expanded["date"] == yesterday
    assert "param" in expanded
    expanded.validate()
    assert req == expanded


@pytest.mark.parametrize(
    "extra_kv",
    [
        {"class": "invalid"},
    ],
)
def test_request_validate(extra_kv):
    request = {
        "class": "od",
        "domain": "g",
        "date": "-1",
        "expver": "0001",
        "step": range(0, 13, 6),
        "levtype": "sfc",
    }
    request.update(extra_kv)
    req = MarsRequest("retrieve", request)
    with pytest.raises(MetKitException):
        req.validate()


@pytest.mark.parametrize(
    "extra_kv, expectation",
    [
        [{"levtype": "pl", "date": "-1"}, pytest.raises(MetKitException)],
        [{"levtype": "sfc", "date": "-1", "type": "em"}, pytest.raises(ValueError)],
        [{"levtype": "sfc", "date": "20230101"}, does_not_raise()],
    ],
)
def test_request_merge(extra_kv, expectation):
    request = {
        "class": "od",
        "domain": "g",
        "expver": "0001",
        "step": range(0, 13, 6),
    }
    req = MarsRequest("retrieve", {**request, "date": "-1", "levtype": "sfc"})
    other_req = MarsRequest("retrieve", {**request, **extra_kv})
    with expectation:
        req.merge(other_req)


@pytest.mark.parametrize(
    "verb, updates, expected",
    [["retrieve", {"date": 20230101, "param": 130}, True], ["compute", {}, False]],
)
def test_request_equality(verb, updates, expected):
    init_request = {
        "class": "od",
        "domain": "g",
        "date": "20230101",
        "param": "130",
        "expver": "0001",
        "step": range(0, 13, 6),
    }
    req = MarsRequest("retrieve", init_request)
    second_request = {**init_request, **updates}
    req2 = MarsRequest(verb, second_request)
    assert (req == req2) == expected
