ecbuild_add_test( TARGET    "metkit_test_gribjump"
SOURCES   "test_gribjump.cc"
INCLUDES  "${ECKIT_INCLUDE_DIRS}"
ENVIRONMENT   "${metkit_env}"
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
NO_AS_NEEDED
LIBS      metkit)