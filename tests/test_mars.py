from datetime import datetime, timedelta
import pytest

from metkit import parse_mars_request
from metkit.mars import Request

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
	target=fileset:test.grib,
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
	target=fileset:test.grib,
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


@pytest.mark.parametrize(
    "req_str, length, steps",
    [
        [request, 2, 5],
        ["retrieve,class=od,date=-1,time=12,param=129,step=12,target=test.grib", 1, 1],
    ],
)
def test_parse_string(req_str, length, steps):
    requests = parse_mars_request(req_str)
    assert len(requests) == length
    for req in requests:
        assert req.num_values("step") == steps


def test_empty_request(tmpdir):
    request_file = f"{tmpdir}/requests"
    with open(request_file, "w") as f:
        f.write("")
    requests = parse_mars_request(open(request_file, "r"))
    assert len(requests) == 0


def test_new_request():
    req = Request("retrieve")
    assert req.verb() == "retrieve"


def test_request_from_dict():
    original_dict = {
        "class": "od",
        "domain": "g",
        "expver": "0001",
        "step": range(0, 13, 6),
    }
    req = Request.from_dict("retrieve", original_dict)
    for name, value in original_dict.items():
        if name == "step":
            assert list(map(str, value)) == req[name]
        else:
            assert value == req[name]
    assert req.verb() == "retrieve"


def test_request_from_expand():
    req = Request.from_dict(
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
