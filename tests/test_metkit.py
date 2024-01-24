from metkit import parse_mars_request

request = """
retrieve,
    class=od,
    domain=g,
    expver=0001,
    levtype=sfc,
    stream=enfo,
    date=20240103,            
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
    date=20240103,            
    time=12,                 
    param=129, 
    levelist=500,              
	grid=O640,                
	step=0/to/24/by/6,               
	target=fileset:test.grib,
	type=em
"""


def test_parse_request(tmpdir):
    requests_file = f"{tmpdir}/requests"
    with open(requests_file, "w") as f:
        f.write(request)
    requests = parse_mars_request(requests_file)
    assert len(requests) == 2
    for req in requests:
        assert req.verb == "retrieve"
        assert len(req["step"]) == 5
    assert len(requests[0]) == 12
    assert len(requests[1]) == 13


def test_empty_request(tmpdir):
    requests_file = f"{tmpdir}/requests"
    with open(requests_file, "w") as f:
        f.write("")
    requests = parse_mars_request(requests_file)
    assert len(requests) == 0
