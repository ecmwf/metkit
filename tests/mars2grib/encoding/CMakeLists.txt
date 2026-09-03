# ignore list goes here
# Add source file names (relative to this directory) to skip auto-registration.
# Example:
#   list( APPEND MARS2GRIB_ENCODING_TEST_IGNORE some_broken_case.cc )
set( MARS2GRIB_ENCODING_TEST_IGNORE "" )


# for *.cc file in this directory, and not in ignore list
file( GLOB MARS2GRIB_ENCODING_TEST_SOURCES
      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
      CONFIGURE_DEPENDS
      "*.cc" )

list( SORT MARS2GRIB_ENCODING_TEST_SOURCES )

foreach( _src IN LISTS MARS2GRIB_ENCODING_TEST_SOURCES )
    if( _src IN_LIST MARS2GRIB_ENCODING_TEST_IGNORE )
        continue()
    endif()

    get_filename_component( _name "${_src}" NAME_WE )

    ecbuild_add_test( TARGET        mars2grib_test_encoding_${_name}
                      CONDITION     HAVE_MARS2GRIB
                      INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                      SOURCES       ${_src}
                      LIBS          metkit
                      NO_AS_NEEDED
    )
endforeach()
