#!/usr/bin/env bash
set -ex

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
version_str=$(cat "${repo_root}/VERSION")
output_dir=${DOCBUILD_OUTPUT:-doc-build}
doxygen_output_dir="${output_dir}/doxygen"
# The public API surface Doxygen parses: the C API header plus the main
# high-level C++ entry point (the MarsRequest class), which live under
# src/metkit/api/ and src/metkit/mars/ respectively.
doxygen_input_dir="${repo_root}/src/metkit/api/metkit_c.h \
${repo_root}/src/metkit/mars/MarsRequest.h"

doxygen_executable="${DOXYGEN_EXECUTABLE:-doxygen}"
sphinx_executable="${SPHINX_EXECUTABLE:-sphinx-build}"

# Substitute CMake @VARIABLE@ placeholders in Doxyfile.in
mkdir -p "${doxygen_output_dir}"
sed \
    -e "s|@DOXYGEN_OUTPUT_DIR@|${doxygen_output_dir}|g" \
    -e "s|@DOXYGEN_INPUT_DIR@|${doxygen_input_dir}|g" \
    -e "s|@METKIT_VERSION@|${version_str}|g" \
    "${script_dir}/Doxyfile.in" > "${output_dir}/Doxyfile"

${doxygen_executable} "${output_dir}/Doxyfile"

# Stage the Doxygen-generated HTML so Sphinx copies it verbatim into the site
# under "doxygen/" (surfaced by the "Doxygen" section).
extra_dir="${output_dir}/extra"
rm -rf "${extra_dir}/doxygen"
mkdir -p "${extra_dir}"
cp -R "${doxygen_output_dir}/html" "${extra_dir}/doxygen"

export DOXYGEN_HTML_EXTRA_DIR="$(cd "${extra_dir}" && pwd)"
export DOXYGEN_XML_DIR="$(cd "${doxygen_output_dir}/xml" && pwd)"

"${sphinx_executable}" -j auto -E -a -T \
    -Dbreathe_projects.Metkit="${DOXYGEN_XML_DIR}" \
    -Dversion="${version_str}" \
    -Drelease="${version_str}" \
    "${script_dir}" "${output_dir}/sphinx"
