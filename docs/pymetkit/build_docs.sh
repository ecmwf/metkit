#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0
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
