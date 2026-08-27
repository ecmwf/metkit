import logging
import warnings
from typing import Any, overload

from pymetkit.pymetkit_type import MarsRequest

from ._internal import Mars2Grib as _Mars2Grib

logger = logging.getLogger(__name__)


class Mars2Grib:
    """
    Mars2Grib encoder object
    """

    def __init__(self, options: dict | None = None):
        self._mars2grib = _Mars2Grib() if options is None else Mars2Grib(options)

    @overload
    def encode(self, values: list[float], mars: MarsRequest, misc: MarsRequest | None = None) -> bytes: ...
    @overload
    def encode(self, values: list[float], mars: dict[Any, Any], misc: dict[Any, Any] | None = None) -> bytes: ...

    def encode(self, values: list[float], mars, misc=None) -> bytes:
        """
        Encode values, described by mars and (optionally) misc dictionaries, as a GRIB2 message

        Note:
        -----
        If you plan to encode the output of Mars2Mars, call its convert method with a typed
        python dictionary, as the types are internally needed. Hand the result of that operation
        to this encode method (it currently doesn't support MarsRequests).

        :param values: Values to encode
        :type values: list[float]
        :param mars: Mars keys describing the values
        :type mars: dict
        :param misc: Miscellaneous (non-mars) keys describing the values, may be empty
        :type misc: dict
        :return: Encoded GRIB2 message as bytes object
        :rtype: bytes
        """
        if isinstance(mars, MarsRequest) and (isinstance(misc, MarsRequest) or misc is None):
            # mars_internal = mars._internal
            # misc_internal = MarsRequest("retrieve", {})._internal if misc is None else misc._internal
            # return self._mars2grib.encode(values, mars_internal, misc_internal)
            raise ValueError("Mars2Grib:encode:: Encoding of MarsRequests is currently not supported.")
        elif isinstance(mars, dict) and (isinstance(misc, dict) or misc is None):
            misc_internal = {} if misc is None else misc
            try:
                return self._mars2grib.encode(values, mars, misc_internal)
            except RuntimeError:
                warnings.warn(
                    "Mars2Grib::encode: Encountered an issue with the type found in the dictionary. "
                    "If you handed in the result of Mars2Mars, make sure to call that function with a typed python dict. "
                    "Hand the result to the encode method, afterwards.",
                    stacklevel=2,
                )
                raise
        else:
            raise ValueError("Mars2Grib:encode:: mars and misc have to be the same object type: MarsRequest or dict")
