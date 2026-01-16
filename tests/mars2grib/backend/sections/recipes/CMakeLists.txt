ecbuild_add_test(

    TARGET
        mars2grib-encoding-recipes

    SOURCES
        mars2grib-encoding-recipes.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit

)
