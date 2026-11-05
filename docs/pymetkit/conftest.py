# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Sybil configuration: execute the code examples in the documentation as tests.

Run with ``pytest`` from this directory once ``pymetkit`` is importable (i.e. the
``metkit`` library is discoverable by ``findlibs``).
"""

from doctest import ELLIPSIS

import pymetkit
from sybil import Sybil
from sybil.parsers.rest import DocTestParser, PythonCodeBlockParser


def sybil_setup(namespace):
    namespace["pymetkit"] = pymetkit


pytest_collect_file = Sybil(
    parsers=[
        DocTestParser(optionflags=ELLIPSIS),
        PythonCodeBlockParser(),
    ],
    patterns=["*.rst", "*.py"],
    setup=sybil_setup,
).pytest()
