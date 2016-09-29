# (C) Copyright 1996-2016 ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

include_directories(${ECKIT_INCLUDE_DIRS})

ecbuild_get_test_multidata( TARGET grib_get_data
                            DIRNAME grib_api/data
                            NAMES latlon.grib )

ecbuild_add_test( TARGET        grib_test_emosfile
                  ARGS          --log_level=message
                  BOOST
                  SOURCES       test_emosfile.cc
                  LIBS          grib
                  TEST_DEPENDS  grib_get_data
)
