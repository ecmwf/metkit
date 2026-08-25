# ---- Per-file copy commands (tracked: re-copy when source changes) ----
# Each add_custom_command pairs one source file with its staging destination.
# cmake re-runs the copy whenever the source is newer than the destination.
set(_pymetkit_py_sources
    __init__.py
    __main__.py
    _internal/__init__.py
    _internal/pymetkit_internal.py
    pymetkit.py
    pymetkit_type.py
)

# A module that exists in src/ but is missing from the list above is silently absent from
# the staging tree and the wheel: nothing fails, the import just goes missing at runtime.
# Fail at configure time instead.
file(GLOB _pymetkit_py_found
     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
     "*.py"
     "_internal/*.py"
     )
foreach(_found IN LISTS _pymetkit_py_found)
    if(NOT "${_found}" IN_LIST _pymetkit_py_sources)
        message(FATAL_ERROR
            "${_found} is not listed in _pymetkit_py_sources (src/CMakeLists.txt), so it "
            "would never reach the pymetkit staging tree or the wheel. Add it there.")
    endif()
endforeach()

set(_pymetkit_staged_py_files)

add_subdirectory(experimental)

foreach(_rel IN LISTS _pymetkit_py_sources)
    set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${_rel}")
    set(_dst "${PYMETKIT_STAGING}/pymetkit/${_rel}")
    add_custom_command(
        OUTPUT  "${_dst}"
        COMMAND ${CMAKE_COMMAND} -E copy "${_src}" "${_dst}"
        DEPENDS "${_src}"
        COMMENT "Copying ${_rel} to pymetkit staging..."
    )
    list(APPEND _pymetkit_staged_py_files "${_dst}")
endforeach()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/pymetkit_setup.py.in
    ${PYMETKIT_STAGING}/setup.py
    @ONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/pymetkit_setup.cfg.in
    ${PYMETKIT_STAGING}/setup.cfg
    @ONLY
)

# A staging copy only runs if a target consumes its output, and an output can
# only be consumed inside the directory that declares it. Hence one target per
# staging directory, chained so that pymetkit drives them all.
add_custom_target(pymetkit DEPENDS ${_pymetkit_staged_py_files})
add_dependencies(pymetkit pymetkit-experimental)
