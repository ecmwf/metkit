add_subdirectory( metkit )
add_subdirectory( tools )

if( HAVE_EXPERIMENTAL )
    add_subdirectory( experimental )
endif()

if( HAVE_PYTHON_METKIT_INTERFACE )
    # We create the complete python package layout at this location.
    # This allows us to run python wheel creation at this path and
    # to put this path on the PYTHONPATH to allow direct use of
    # pymetkit, e.g. for testing or local exploration.
    set(PYMETKIT_STAGING "${CMAKE_BINARY_DIR}/pymetkit-python-package-staging")
    file(MAKE_DIRECTORY "${PYMETKIT_STAGING}")
    file(CREATE_LINK
        "${CMAKE_CURRENT_SOURCE_DIR}/pymetkit"
        "${PYMETKIT_STAGING}/pymetkit" SYMBOLIC
    )
    file(CREATE_LINK
        "${CMAKE_CURRENT_SOURCE_DIR}/pymetkit/README.md"
        "${PYMETKIT_STAGING}/README.md"
        SYMBOLIC RESULT _ignored)
    file(CREATE_LINK
        "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
        "${PYMETKIT_STAGING}/LICENSE"
        SYMBOLIC RESULT _ignored)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/pymetkit_setup.py.in
        ${PYMETKIT_STAGING}/setup.py
        @ONLY
    )
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/pymetkit_setup.cfg.in
        ${PYMETKIT_STAGING}/setup.cfg
        @ONLY
    )
    add_subdirectory(pymetkit_bindings)
    file(GLOB_RECURSE
        _pymetkit_package_files
        "pymetkit/*.py"
        "pymetkit/_internal/*.py"
    )
    list(APPEND _pymetkit_package_files
        "${CMAKE_CURRENT_SOURCE_DIR}/pymetkit/README.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
    )
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/pymetkit.wheel.stamp
        COMMAND ${Python_EXECUTABLE} -m build --wheel ${PYMETKIT_STAGING} -o .
        COMMAND ${CMAKE_COMMAND} -E touch pymetkit.wheel.stamp
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${_pymetkit_package_files} pymetkit_bindings
        COMMENT "Building Python wheel for pymetkit..."
    )
    add_custom_target(pymetkit-wheel ALL DEPENDS ${CMAKE_BINARY_DIR}/pymetkit.wheel.stamp)
endif()
