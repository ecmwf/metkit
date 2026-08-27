from pymetkit.pymetkit import MarsRequest
from pymetkit.pymetkit_type import UserInputMapper

from ._internal import Mars2Mars as _Mars2Mars


class Mars2Mars:
    """
    Mars2Mars

    """

    def __init__(self, options: dict = {}):
        self._mars2mars = _Mars2Mars(options)

    def convert(self, mars_request: MarsRequest) -> tuple[dict[str, list[str]], dict[str, list[str]]]:
        """
        Convert a mars request

        :return: Tuple of misc and mars dictionary

        """
        return self._mars2mars.convert(mars_request._internal)
