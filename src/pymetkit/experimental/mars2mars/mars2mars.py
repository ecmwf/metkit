from ._internal import Mars2Mars as _Mars2Mars


class Mars2Mars:
    """
    Mars2Mars
    """

    def __init__(self, options: dict = {}):
        self._mars2mars = _Mars2Mars(options)
