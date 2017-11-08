# (C) Copyright 1996-2017 ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

ecbuild_get_test_multidata( TARGET grib_get_data
                            DIRNAME grib_api/data
                            NAMES latlon.grib )

ecbuild_add_test( TARGET        metkit_test_emosfile
                  ARGS          --log_level=message
                  CONDITION     HAVE_GRIB
                  INCLUDES      "${ECKIT_INCLUDE_DIRS}"
                  SOURCES       test_emosfile.cc
                  LIBS          metkit
                  TEST_DEPENDS  grib_get_data
)

if( IS_DIRECTORY "${CMAKE_BINARY_DIR}/share/eccodes/definitions" )
  set( eccodes_env "ECCODES_DEFINITION_PATH=${CMAKE_BINARY_DIR}/share/eccodes/definitions" )
endif()

ecbuild_add_test( TARGET        metkit_test_multihandle
                  CONDITION     HAVE_GRIB
                  INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                  SOURCES       test_multihandle.cc
                  LIBS          metkit
                  TEST_DEPENDS  grib_get_data
                  ENVIRONMENT   "${eccodes_env}"
)

ecbuild_add_test( TARGET    test_typesfactory
                  SOURCES   test_typesfactory.cc
                  INCLUDES  "${ECKIT_INCLUDE_DIRS}"
                  LIBS      metkit )

ecbuild_add_test( TARGET    test_expand
                  SOURCES   test_expand.cc
                  INCLUDES  "${ECKIT_INCLUDE_DIRS}"
                  LIBS      metkit)


# if ( HAVE_NETCDF )
#    add_subdirectory(netcdf)
# endif()
