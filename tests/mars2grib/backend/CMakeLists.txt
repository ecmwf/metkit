# add_subdirectory(sections)
# add_subdirectory(concepts)


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
#ecbuild_add_test(
#    TARGET
#        mars2grib-test-bitset
#
#    SOURCES
#        mars2grib-test-bitset.cc
#
#    NO_AS_NEEDED
#
#    LIBS
#        eckit
#        metkit
#)

# ecbuild_add_test(
#     TARGET
#         mars2grib-test-concept-index
# 
#     SOURCES
#         mars2grib-test-concept-index.cc
# 
#     NO_AS_NEEDED
# 
#     LIBS
#         eckit
#         metkit
# )

ecbuild_add_test(
    TARGET
        mars2grib-test-concept-index01

    SOURCES
        mars2grib-test-concept-index01.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(
    TARGET
        mars2grib-test-concept-index02

    SOURCES
        mars2grib-test-concept-index02.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(
    TARGET
        mars2grib-test-selector

    SOURCES
        mars2grib-test-selector.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)


ecbuild_add_test(
    TARGET
        mars2grib-test-compression-mask

    SOURCES
        mars2grib-test-compression-mask.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(
    TARGET
        mars2grib-test-search

    SOURCES
        mars2grib-test-search.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(
    TARGET
        mars2grib-test-match

    SOURCES
        mars2grib-test-match.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(
    TARGET
        mars2grib-test-recipe

    SOURCES
        mars2grib-test-recipe.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(
    TARGET
        mars2grib-test-recipes

    SOURCES
        mars2grib-test-recipes.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(
    TARGET
        mars2grib-test-normalization

    SOURCES
        mars2grib-test-normalization.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

# ecbuild_add_test(
#     TARGET
#         mars2grib-test-recipes
# 
#     SOURCES
#         mars2grib-test-recipes.cc
# 
#     NO_AS_NEEDED
# 
#     LIBS
#         eckit
#         metkit
# )


#ecbuild_add_test(
#    TARGET
#        mars2grib-test-recipe
#
#    SOURCES
#        mars2grib-test-recipe.cc
#
#    NO_AS_NEEDED
#
#    LIBS
#        eckit
#        metkit
#)

# ecbuild_add_test(
#     TARGET
#         mars2grib-check-configuration-modifier
# 
#     SOURCES
#         mars2grib-check-configuration-modifier.cc
# 
#     NO_AS_NEEDED
# 
#     LIBS
#         eckit
#         metkit
# )