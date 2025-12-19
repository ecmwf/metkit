
ecbuild_add_executable(
    TARGET   compare-mars-to-encoder
    SOURCES  compare-mars-to-encoder.cc
    INCLUDES ${ECKIT_INCLUDE_DIRS}
    LIBS     metkit eckit_option eckit
    NO_AS_NEEDED
)
