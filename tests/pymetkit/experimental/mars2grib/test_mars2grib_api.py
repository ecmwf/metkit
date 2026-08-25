import pytest

from pymetkit.experimental.mars2grib import Mars2Grib
from pymetkit.pymetkit_type import MarsRequest


def test_encode_dict():
    encoder = Mars2Grib()

    mars = {
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

    vals = [237.15] * 200

    encoder.encode(vals, mars)


def test_encode_dict_string():
    encoder = Mars2Grib()

    mars = {
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

    vals = [237.15] * 200

    # We don't support dict[str, str] yet
    with pytest.raises(RuntimeError):
        encoder.encode(vals, mars)


def test_encode_mars_request():
    encoder = Mars2Grib()

    mars_request = MarsRequest(
        "retrieve",
        {
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
        },
    )

    vals = [237.15] * 200

    with pytest.raises(ValueError, match="Mars2Grib::encode: Currently metkit::mars::MarsRequest is not supported."):
        encoder.encode(vals, mars_request)


def test_encode_mixed_types():
    encoder = Mars2Grib()

    mars = MarsRequest(
        "retrieve",
        {
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
        },
    )

    vals = [237.15] * 200

    with pytest.raises(ValueError, match="mars and misc have to be the same object type"):
        encoder.encode(vals, mars, misc={})
