# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

list(APPEND metkit_env "METKIT_HOME=${CMAKE_BINARY_DIR}")

if( DEFINED ECCODES_DEFINITION_PATH )
  list(APPEND metkit_env "ECCODES_DEFINITION_PATH=${ECCODES_DEFINITION_PATH}")
elseif( IS_DIRECTORY "${CMAKE_BINARY_DIR}/share/eccodes/definitions" )
  list(APPEND metkit_env "ECCODES_DEFINITION_PATH=${CMAKE_BINARY_DIR}/share/eccodes/definitions")
endif()


ecbuild_get_test_multidata( TARGET grib_get_data
                            DIRNAME grib-api/test-data/data
                            NAMES latlon.grib
                                  synthetic_2msgs.grib )

ecbuild_get_test_multidata( TARGET metkit_get_odb_data
                            NAMES multiodb.odb
                                  multiodb2.odb
                            NOCHECK )

add_custom_target(soft_link_expand_test_data ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/expand" "${CMAKE_CURRENT_BINARY_DIR}/expand")

ecbuild_add_test( TARGET        metkit_test_multihandle
                  CONDITION     HAVE_GRIB
                  INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                  SOURCES       test_multihandle.cc
                  LIBS          metkit
                  NO_AS_NEEDED
                  TEST_DEPENDS  grib_get_data
                  ENVIRONMENT   "${metkit_env}"
)

ecbuild_add_test( TARGET      "metkit_test_codes_decoder"
                  CONDITION   HAVE_GRIB OR HAVE_BUFR
                  SOURCES     "test_codes_decoder.cc"
                  INCLUDES    "${ECKIT_INCLUDE_DIRS}" "${ECCODES_INCLUDE_DIR}"
                  LIBS        metkit
                  NO_AS_NEEDED
                  ENVIRONMENT "${metkit_env}"
)

ecbuild_add_test( TARGET        metkit_test_odbsplitter
                  CONDITION     HAVE_ODB
                  SOURCES       test_odbsplitter.cc
                  INCLUDES      "${ECKIT_INCLUDE_DIRS}"
                  TEST_DEPENDS  metkit_get_odb_data
                  NO_AS_NEEDED
                  LIBS          metkit )

foreach( test
        c_api
        codes_api
        context
        date
        expand
        filter
        hypercube
        integer_range
        matcher
        language
        mars_language
        mars_language_strict
        obstype
        param_axis
        request
        step
        steprange_axis
        time
        type_levelist
        typesfactory )
    ecbuild_add_test( TARGET       "metkit_test_${test}"
                      SOURCES      "test_${test}.cc"
                      INCLUDES     "${ECKIT_INCLUDE_DIRS}"
                      ENVIRONMENT  "${metkit_env}"
                      NO_AS_NEEDED
                      LIBS         metkit)
endforeach()

# Compile C test
ecbuild_add_test( TARGET        metkit_test_c_compiled
                  SOURCES       test_c_api.c
                  INCLUDES      "${ECKIT_INCLUDE_DIRS}"
                  LIBS          metkit
                  NO_AS_NEEDED
                  ENVIRONMENT   "${metkit_env}")

# if ( HAVE_NETCDF )
#    add_subdirectory(netcdf)
# endif()

add_subdirectory(regressions)
add_subdirectory(marsgen)
add_subdirectory(mars2grib)
