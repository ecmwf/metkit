import pytest
import logging

from pymetkit.experimental.mars2mars import Mars2Mars
from pymetkit.experimental.mars2grib import Mars2Grib
from pymetkit.pymetkit_type import MarsRequest

logger = logging.getLogger(__name__)


def test_typed_dict_mars2mars_then_mars2grib():

    request = {
        "origin": "ecmf",
        "class": "od",
        "stream": "oper",
        "type": "fc",
        "expver": "0001",
        "grid": "N200",
        "packing": "ccsds",
        "param": 130,
        "levtype": "hl",
        "levelist": 2,
        "date": 20260205,
        "time": 000000,
        "step": 0,
    }

    converter = Mars2Mars()
    [request, misc] = converter.convert(mars_request=request)

    import json

    logger.debug(json.dumps(request))
    logger.debug(json.dumps(misc))

    encoder = Mars2Grib()

    vals = [237.15] * 200
    encoder.encode(vals, request)


def test_mars_request_mars2mars_then_mars2grib():

    request = {
        "origin": "ecmf",
        "class": "od",
        "stream": "oper",
        "type": "fc",
        "expver": "0001",
        "grid": "N200",
        "packing": "ccsds",
        "param": 130,
        "levtype": "hl",
        "levelist": 2,
        "date": 20260205,
        "time": 000000,
        "step": 0,
    }

    converter = Mars2Mars()
    [request, misc] = converter.convert(mars_request=MarsRequest("retrieve", request))

    import json

    logger.debug(json.dumps(request))
    logger.debug(json.dumps(misc))

    encoder = Mars2Grib()

    vals = [237.15] * 200
    with pytest.raises(ValueError, match="not supported"):
        encoder.encode(vals, request)
