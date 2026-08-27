from pymetkit.experimental.mars2grib import Mars2Grib


def test_encode():
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
