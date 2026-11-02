ecbuild_add_test( TARGET        mars2grib_test_strip_plan
                  CONDITION     HAVE_MARS2GRIB AND eckit_HAVE_ECKIT_BLOCKFILE
                  SOURCES       test_strip_plan.cc
                                ${PROJECT_SOURCE_DIR}/src/metkit/mars2grib/testing-utils/TestRecord.cc
                                ${PROJECT_SOURCE_DIR}/src/metkit/mars2grib/testing-utils/TestCaseLabels.cc
                                ${PROJECT_SOURCE_DIR}/src/metkit/mars2grib/testing-utils/strip/Envelope.cc
                                ${PROJECT_SOURCE_DIR}/src/metkit/mars2grib/testing-utils/strip/Plan.cc
                  LIBS          metkit eckit_blockfile
                  NO_AS_NEEDED )
