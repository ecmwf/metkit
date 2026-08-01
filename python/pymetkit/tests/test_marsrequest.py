from datetime import datetime, timedelta
from contextlib import nullcontext as does_not_raise
import pytest

from pymetkit import parse_mars_request, expand_key, MarsRequest, MetKitException

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

# @todo: [1] no longer raises an exception. Disable until METK-126 is resolved.
@pytest.mark.parametrize(
    "req_str, length, steps, strict, expectation",
    [
        [request, 2, 5, False, does_not_raise()],
        # [request, 2, 5, True, pytest.raises(MetKitException)],
        [
            "retrieve,class=od,date=-1,time=12,param=129,step=12,target=test.grib",
            1,
            1,
            False,
            does_not_raise(),
        ],
    ],
)
def test_parse_string(req_str, length, steps, strict, expectation):
    with expectation:
        requests = parse_mars_request(req_str, strict)
        assert len(requests) == length
        for req in requests:
            assert req.num_values("step") == steps


def test_empty_request(tmpdir):
    request_file = f"{tmpdir}/requests"
    with open(request_file, "w") as f:
        f.write("")
    requests = parse_mars_request(open(request_file, "r"))
    assert len(requests) == 0


@pytest.mark.parametrize(
    "keyword, value, kwargs, expected",
    [
        ["step", "1/to/10/by/1", {}, [str(i) for i in range(1, 11)]],
        ["step", [0, 6, 12], {}, ["0", "6", "12"]],
        ["time", "6/to/18/by/6", {}, ["0600", "1200", "1800"]],
        ["date", "-1", {}, [yesterday]],
        # param resolves via full multi-pass expansion using context
        ["param", "t", {"context": {"levtype": "pl"}}, ["130"]],
        # context-sensitive keyword: levelist depends on levtype
        [
            "levelist",
            "1000/to/850/by/50",
            {"context": {"levtype": "pl"}},
            ["1000", "950", "900", "850"],
        ],
    ],
)
def test_expand_key(keyword, value, kwargs, expected):
    assert expand_key(keyword, value, **kwargs) == expected


def test_expand_key_param_resolves_with_context():
    # 'param' is resolved transparently; context drives the result.
    sfc = {"class": "od", "stream": "oper", "type": "an", "levtype": "sfc"}
    assert expand_key("param", "t", context=sfc) == ["164"]


def test_expand_key_context_marsrequest():
    context = MarsRequest("retrieve", levtype="pl")
    assert expand_key("levelist", "500/to/300/by/100", context=context) == [
        "500",
        "400",
        "300",
    ]


def test_expand_key_strict_raises():
    with pytest.raises(MetKitException):
        expand_key("time", "notatime", strict=True)


def test_new_request():
    req = MarsRequest("retrieve")
    assert req.verb() == "retrieve"

    req = MarsRequest("request", class_="od", type="pf", date=["20200101", "20200102"])
    assert req["class"] == "od"
    assert req["type"] == "pf"
    assert req["date"] == ["20200101", "20200102"]


def test_request_from_expand():
    req = MarsRequest(
        "retrieve",
        **{
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

# @todo: [0] and [1] no longer raise an exception. Disable until METK-126 is resolved.
@pytest.mark.parametrize(
    "extra_kv",
    [
        # {"levelist": [500]},
        # {"type": "cf", "number": [1, 2]},
        {"class": "invalid"}
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
    req = MarsRequest("retrieve", **request)
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
    req = MarsRequest("retrieve", **request, date="-1", levtype="sfc")
    other_req = MarsRequest("retrieve", **request, **extra_kv)
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
    req = MarsRequest(
        "retrieve",
        **init_request,
    )
    second_request = {**init_request, **updates}
    req2 = MarsRequest(verb, **second_request)
    assert (req == req2) == expected
