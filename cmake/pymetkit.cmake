# (C) Copyright 2026- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

# Stages, builds and installs the pymetkit wheel, from metkit's own build
# or from python/pymetkit against an installed metkit.

set(_pymetkit_src "${CMAKE_CURRENT_LIST_DIR}/../src")

# We create the complete python package layout at this location.
# This allows us to run python wheel creation at this path and
# to put this path on the PYTHONPATH to allow direct use of
# pymetkit, e.g. for testing or local exploration.
set(PYMETKIT_STAGING "${CMAKE_BINARY_DIR}/pymetkit-python-package-staging")

# Wipe any layout left over from a previous configuration so stale files
# cannot reach the wheel. file(REMOVE_RECURSE) does not follow symlinks.
file(REMOVE_RECURSE "${PYMETKIT_STAGING}")
file(MAKE_DIRECTORY "${PYMETKIT_STAGING}")

# Symlink the whole package directory so Python edits are immediately
# reflected in the staging tree without a rebuild step.
file(CREATE_LINK
    "${_pymetkit_src}/pymetkit"
    "${PYMETKIT_STAGING}/pymetkit"
    SYMBOLIC
)

# Symlink wheel metadata files into the staging root.
file(CREATE_LINK
    "${_pymetkit_src}/pymetkit/README.md"
    "${PYMETKIT_STAGING}/README.md"
    SYMBOLIC RESULT _ignored
)
file(CREATE_LINK
    "${_pymetkit_src}/../LICENSE"
    "${PYMETKIT_STAGING}/LICENSE"
    SYMBOLIC RESULT _ignored
)

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/pymetkit_setup.py.in
    ${PYMETKIT_STAGING}/setup.py
    @ONLY
)
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/pymetkit_setup.cfg.in
    ${PYMETKIT_STAGING}/setup.cfg
    @ONLY
)

set(PYMETKIT_STAGING_PYMETKIT "${PYMETKIT_STAGING}/pymetkit")
set(PYMETKIT_STAGING_PYMETKIT_INTERNAL "${PYMETKIT_STAGING}/pymetkit/_internal")
set(PYMETKIT_STAGING_PYMETKIT_EXPERIMENTAL "${PYMETKIT_STAGING}/pymetkit/experimental")

add_subdirectory(${_pymetkit_src}/pymetkit ${CMAKE_CURRENT_BINARY_DIR}/pymetkit)
add_subdirectory(${_pymetkit_src}/pymetkit_bindings ${CMAKE_CURRENT_BINARY_DIR}/pymetkit_bindings)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/pymetkit.wheel.stamp
    COMMAND ${Python_EXECUTABLE} -m build --wheel ${PYMETKIT_STAGING} -o .
    COMMAND ${CMAKE_COMMAND} -E touch pymetkit.wheel.stamp
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS pymetkit pymetkit_bindings
    COMMENT "Building Python wheel for pymetkit..."
)
add_custom_target(pymetkit-wheel ALL DEPENDS ${CMAKE_BINARY_DIR}/pymetkit.wheel.stamp)

install(CODE "
    file(GLOB _whl \"${CMAKE_BINARY_DIR}/pymetkit-*.whl\")
    file(INSTALL \${_whl} DESTINATION \"\${CMAKE_INSTALL_PREFIX}\")
")
