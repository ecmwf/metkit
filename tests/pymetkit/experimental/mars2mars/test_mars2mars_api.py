import pytest
import logging
from pathlib import Path

logger = logging.getLogger(__name__)

from pymetkit import MarsRequest, MarsSelection
from pymetkit.experimental.mars2mars import Mars2Mars


def test_convert_mars_request_to_json():

    mars_selection: MarsSelection = {
        "class": "od",
        "stream": "oper",
        "type": "fc",
        "expver": "1",
        "date": "2026-02-09",
        "time": "00:00:00",
        "levtype": "sfc",
        "param": "261018",
        "step": "0",
    }

    request = MarsRequest("retrieve", mars_selection).expand()

    [mars, misc] = Mars2Mars().convert(mars_request=request)

    import json

    logger.debug(json.dumps(mars))
    logger.debug(json.dumps(misc))


def test_convert_typed_dict_to_json():

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

    [request, misc] = Mars2Mars().convert(mars_request=request)

    import json

    logger.debug(json.dumps(request))
    logger.debug(json.dumps(misc))


def test_convert_string_dict_to_json():

    request = {
        "origin": "ecmf",
        "class": "od",
        "stream": "oper",
        "type": "fc",
        "expver": "0001",
        "grid": "N200",
        "packing": "ccsds",
        "param": "130",
        "levtype": "hl",
        "levelist": "2",
        "date": "20260205",
        "time": "000000",
        "step": "0",
    }

    with pytest.raises(RuntimeError, match="Key `param` is not of expected type `long` for dictionary"):
        [request, misc] = Mars2Mars().convert(mars_request=request)

        import json

        logger.debug(json.dumps(request))
        logger.debug(json.dumps(misc))


def test_flatten_convert_append_to_json(data_path):

    from pymetkit import parse_mars_request

    with (data_path / "mars2mars-test-requests.mars").open() as file:
        requests = parse_mars_request(file)

    assert len(requests) != 0, "Expected requests from request file"

    splitting_keys = [
        "class",
        "stream",
        "type",
        "expver",
        "levtype",
        "param",
        "levelist",
        "step",
        "date",
        "time",
        "number",
        "chem",
        "wavelength",
        "timespan",
        "hdate",
        "htime",
    ]

    mars2mars = Mars2Mars()

    convertedPoints = []

    for req in requests:
        expanded = req.expand()
        identifiers = expanded.split(splitting_keys)

        assert len(identifiers) != 0, "Expecting points not to be empty"

        logger.debug(f"Points: {identifiers[0:2]} ... {identifiers[-1]}")

        convertedPoints.extend(
            [mars for [mars, _] in [mars2mars.convert(mars_request=identifier) for identifier in identifiers]]
        )

    import json

    logger.debug(json.dumps(convertedPoints))
