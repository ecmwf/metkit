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
                            NAMES latlon.grib )

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
                  TEST_DEPENDS  grib_get_data
                  ENVIRONMENT   "${metkit_env}"
)

ecbuild_add_test( TARGET    metkit_test_typesfactory
                  SOURCES   test_typesfactory.cc
                  INCLUDES  "${ECKIT_INCLUDE_DIRS}"
                  ENVIRONMENT   "${metkit_env}"
                  LIBS      metkit )

ecbuild_add_test( TARGET    metkit_test_expand
                  SOURCES   test_expand.cc
                  INCLUDES  "${ECKIT_INCLUDE_DIRS}"
                  ENVIRONMENT   "${metkit_env}"
                  LIBS      metkit)

ecbuild_add_test( TARGET    metkit_test_param_axis
                  SOURCES   test_param_axis.cc
                  INCLUDES  "${ECKIT_INCLUDE_DIRS}"
                  ENVIRONMENT   "${metkit_env}"
                  LIBS      metkit)

ecbuild_add_test( TARGET      metkit_test_steprange_axis
                  SOURCES     test_steprange_axis.cc
                  ENVIRONMENT "${metkit_env}"
                  LIBS        metkit )

ecbuild_add_test( TARGET      metkit_test_time
                  SOURCES     test_time.cc
                  LIBS        metkit )

ecbuild_add_test( TARGET      metkit_test_hypercube
                  SOURCES     test_hypercube.cc
                  LIBS        metkit )

ecbuild_add_test( TARGET      metkit_test_codes_decoder
                  SOURCES     test_codes_decoder.cc
                  LIBS        metkit )

# if ( HAVE_NETCDF )
#    add_subdirectory(netcdf)
# endif()

add_subdirectory(regressions)
