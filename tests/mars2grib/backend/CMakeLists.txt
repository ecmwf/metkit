add_subdirectory(sections)
add_subdirectory(concepts)


#ecbuild_add_test(
#    TARGET
#        test-encoder
#
#    SOURCES
#        test_encoder.cc
#
#    NO_AS_NEEDED
#
#    LIBS
#        eckit
#        metkit
#)


ecbuild_add_test(
    TARGET
        mars2grib-check-configuration-modifier

    SOURCES
        mars2grib-check-configuration-modifier.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)