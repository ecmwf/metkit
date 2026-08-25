# ---- Per-file copy commands (tracked: re-copy when source changes) ----
# Each add_custom_command pairs one source file with its staging destination.
# cmake re-runs the copy whenever the source is newer than the destination.
set(_pymetkit_mars2mars_py_sources
    __init__.py
    mars2mars.py
    _internal/__init__.py
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
    if(NOT "${_found}" IN_LIST _pymetkit_mars2mars_py_sources)
        message(FATAL_ERROR
            "${_found} is not listed in _metkit_mars2mars_py_sources (src/CMakeLists.txt), so it "
            "would never reach the pymetkit staging tree or the wheel. Add it there.")
    endif()
endforeach()

foreach(_rel IN LISTS _pymetkit_mars2mars_py_sources)
    set(_src "${CMAKE_CURRENT_SOURCE_DIR}/${_rel}")
    set(_dst "${PYMETKIT_STAGING_PYMETKIT_EXPERIMENTAL}/mars2mars/${_rel}")
    add_custom_command(
        OUTPUT  "${_dst}"
        COMMAND ${CMAKE_COMMAND} -E copy "${_src}" "${_dst}"
        DEPENDS "${_src}"
        COMMENT "Copying ${_rel} to pymetkit staging..."
    )
    list(APPEND _pymetkit_mars2mars_staged_py_files  "${_dst}")
endforeach()

# Own target for the copies declared above, chained into pymetkit-experimental.
add_custom_target(pymetkit-mars2mars DEPENDS ${_pymetkit_mars2mars_staged_py_files})

add_subdirectory(_internal)

# The wheel picks up packages by probing the staging tree, so the extension
# module has to be in place before the wheel is built.
add_dependencies(pymetkit-mars2mars mars2mars_bindings)

