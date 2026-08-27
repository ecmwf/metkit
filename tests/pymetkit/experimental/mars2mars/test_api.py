import logging
from pathlib import Path

logger = logging.getLogger(__name__)

from pymetkit import MarsRequest, MarsSelection
from pymetkit.experimental.mars2mars import Mars2Mars


def test_convert_to_json():

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


def test_flatten_convert_append_to_json():

    from pymetkit import parse_mars_request

    with Path("mars2mars-test-requests.mars").open() as file:
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
        points = expanded.split(splitting_keys)

        assert len(points) != 0, "Expecting points not to be empty"

        logger.debug(f"Points: {points[0:2]} ... {points[-1]}")

        convertedPoints.extend([mars for [mars, _] in [mars2mars.convert(mars_request=point) for point in points]])

    import json

    logger.debug(json.dumps(convertedPoints))
