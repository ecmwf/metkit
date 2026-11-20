add_subdirectory( metkit )
add_subdirectory( tools )

if( HAVE_EXPERIMENTAL )
    add_subdirectory( experimental )
endif()

if( HAVE_PYTHON_METKIT_INTERFACE )
    include(${PROJECT_SOURCE_DIR}/cmake/pymetkit.cmake)
endif()
