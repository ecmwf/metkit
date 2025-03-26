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
                            DIRNAME grib_api/data
                            NAMES latlon.grib
                                  synthetic_2msgs.grib )

ecbuild_get_test_multidata( TARGET metkit_get_odb_data
                            NAMES multiodb.odb
                                  multiodb2.odb )

# ecbuild_add_test( TARGET        metkit_test_emosfile
#                   ARGS          --log_level=message
#                   CONDITION     HAVE_GRIB
#                   INCLUDES      "${ECKIT_INCLUDE_DIRS}"
#                   SOURCES       test_emosfile.cc
#                   ENVIRONMENT   "${metkit_env}"
#                   LIBS          metkit
#                   TEST_DEPENDS  grib_get_data
# )


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

ecbuild_add_test( TARGET        metkit_test_gribhandle
                  CONDITION     HAVE_GRIB
                  SOURCES       test_gribhandle.cc
                  INCLUDES      "${ECKIT_INCLUDE_DIRS}" "${ECCODES_INCLUDE_DIR}"
                  LIBS          metkit
                  TEST_DEPENDS  grib_get_data
                  NO_AS_NEEDED
                  ENVIRONMENT   "${metkit_env}")

list(APPEND testFileSuffixes typesfactory expand request param_axis steprange_axis date time step hypercube type_levelist c_api integer_range )

foreach(test IN LISTS testFileSuffixes)
    ecbuild_add_test( TARGET    "metkit_test_${test}"
                      SOURCES   "test_${test}.cc"
                      INCLUDES  "${ECKIT_INCLUDE_DIRS}"
                      ENVIRONMENT   "${metkit_env}"
                      NO_AS_NEEDED
                      LIBS      metkit)
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
