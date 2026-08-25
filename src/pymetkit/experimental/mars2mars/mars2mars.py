from typing import Any

from pymetkit.pymetkit import MarsRequest

from ._internal import Mars2Mars as _Mars2Mars


class Mars2Mars:
    """
    Mars2Mars

    """

    def __init__(self, options: dict = {}):
        self._mars2mars = _Mars2Mars(options)

    def convert(self, mars_request: MarsRequest | dict[Any, Any]) -> tuple[dict[Any, Any], dict[Any, Any]]:
        """
        Convert a mars request

        Note:
        -----
        If you plan to encode the output of Mars2Mars' convert method, call it with a typed
        python dictionary, as the types are internally needed. Hand the result of that operation
        to this encode method (it currently doesn't support MarsRequests).

        :return: Tuple of misc and mars dictionary

        """
        if isinstance(mars_request, MarsRequest):
            return self._mars2mars.convert(mars_request._internal)

        if isinstance(mars_request, dict):
            return self._mars2mars.convert(mars_request)
