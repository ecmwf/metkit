# for *.cc file in this directory
file( GLOB MARS2GRIB_ENCODING_TEST_SOURCES
      RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
      CONFIGURE_DEPENDS
      "*.cc" )

list( SORT MARS2GRIB_ENCODING_TEST_SOURCES )

foreach( _src IN LISTS MARS2GRIB_ENCODING_TEST_SOURCES )
    get_filename_component( _name "${_src}" NAME_WE )

    ecbuild_add_test( TARGET        mars2grib_test_encoding_${_name}_legacy
                      CONDITION     HAVE_MARS2GRIB
                      INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                      SOURCES       ${_src}
                      LIBS          metkit
                      NO_AS_NEEDED
    )

    ecbuild_add_test( TARGET        mars2grib_test_encoding_${_name}_gridSpec
                      CONDITION     HAVE_MARS2GRIB
                      INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                      SOURCES       ${_src}
                      LIBS          metkit
                      ENVIRONMENT   ECCODES_ECKIT_GEO=1
                      NO_AS_NEEDED
    )
endforeach()
