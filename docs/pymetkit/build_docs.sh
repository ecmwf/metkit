#!/usr/bin/env bash
# (C) Copyright 2025- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.
#
# Build the PyMetKit Sphinx documentation.
#
# Usage: docs/pymetkit/build_docs.sh
#
# The pythonic API is documented via sphinx-autoapi, which parses the sources in
# src/pymetkit statically -- the metkit library does not need to be importable.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT_DIR="${1:-${SCRIPT_DIR}/doc-build/sphinx}"

sphinx-build -j auto -E -a -T -b html "${SCRIPT_DIR}" "${OUT_DIR}"

echo "Documentation built at: ${OUT_DIR}"
