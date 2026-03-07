from .mars2grib_core import init_bindings, Mars2GribCore

class Mars2Grib():
    """
    Mars2Grib encoder object
    """

    def __init__(self, options: dict = {}):
        init_bindings()
        self._mars2grib = Mars2GribCore(options)

    def encode(self, values: list[float], mars: dict, misc: dict = {}) -> bytes:
        """
        Encode values, described by mars and (optionally) misc dictionaries, as a GRIB2 message

        :param values: Values to encode
        :type values: list[float]
        :param mars: Mars keys describing the values
        :type mars: dict
        :param misc: Miscellaneous (non-mars) keys describing the values, may be empty
        :type misc: dict
        :return: Encoded GRIB2 message as bytes object
        :rtype: bytes
        """
        return self._mars2grib.encode(values, mars, misc)
