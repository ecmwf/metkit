# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

ecbuild_add_test( TARGET        metkit_test_mars2grib_pdt_selection
                  CONDITION     HAVE_GRIB
                  INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                  SOURCES       test_pdt_selection.cc
                  LIBS          metkit
                  NO_AS_NEEDED
                  # TEST_DEPENDS  grib_get_data
                  ENVIRONMENT   "${metkit_env}"
)

