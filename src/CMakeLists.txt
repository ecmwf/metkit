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

    # The whole staging tree is generated. Wipe it so a layout left over from an
    # earlier configuration cannot reach the wheel: setup.py probes the tree to
    # decide which packages to include, e.g. it picks up a mars2grib staged
    # before MARS2GRIB_PYTHON was turned off.
    file(REMOVE_RECURSE "${PYMETKIT_STAGING}")

    # Create all staging subdirectories at configure time so build-time copy
    # commands do not need to worry about missing parent directories.
    file(MAKE_DIRECTORY
        "${PYMETKIT_STAGING}"
        "${PYMETKIT_STAGING}/pymetkit"
        "${PYMETKIT_STAGING}/pymetkit/_internal"
        "${PYMETKIT_STAGING}/pymetkit/experimental"
    )

    set(PYMETKIT_STAGING_PYMETKIT "${PYMETKIT_STAGING}/pymetkit")
    set(PYMETKIT_STAGING_PYMETKIT_INTERNAL "${PYMETKIT_STAGING}/pymetkit/_internal")
    set(PYMETKIT_STAGING_PYMETKIT_EXPERIMENTAL "${PYMETKIT_STAGING}/pymetkit/experimental")

    add_subdirectory(pymetkit)
    add_subdirectory(pymetkit_bindings)
    
    # setup.py and setup.cfg read both files from the staging root, so they are
    # staged there rather than inside the package. Declared here because a
    # custom command output can only be consumed in its own directory.
    set(_pymetkit_staging_root_srcs
        "${CMAKE_CURRENT_SOURCE_DIR}/pymetkit/README.md"
        "${PROJECT_SOURCE_DIR}/LICENSE"
    )

    foreach(_src IN LISTS _pymetkit_staging_root_srcs)
        get_filename_component(_name "${_src}" NAME)
        add_custom_command(
            OUTPUT  "${PYMETKIT_STAGING}/${_name}"
            COMMAND ${CMAKE_COMMAND} -E copy "${_src}" "${PYMETKIT_STAGING}/${_name}"
            DEPENDS "${_src}"
            COMMENT "Copying ${_name} to pymetkit staging..."
        )
    endforeach()

    # The pymetkit target pulls in every staged .py file, one target per staging
    # directory. A variable cannot carry that list up to here: set() in a
    # subdirectory does not reach the parent scope.
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/pymetkit.wheel.stamp
        COMMAND ${Python_EXECUTABLE} -m build --wheel ${PYMETKIT_STAGING} -o .
        COMMAND ${CMAKE_COMMAND} -E touch pymetkit.wheel.stamp
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${PYMETKIT_STAGING}/README.md ${PYMETKIT_STAGING}/LICENSE
                pymetkit
                pymetkit_bindings
        COMMENT "Building Python wheel for pymetkit..."
    )
    add_custom_target(pymetkit-wheel ALL DEPENDS ${CMAKE_BINARY_DIR}/pymetkit.wheel.stamp)
endif()

