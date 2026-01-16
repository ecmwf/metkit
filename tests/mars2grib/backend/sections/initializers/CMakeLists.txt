
ecbuild_add_test(
  TARGET
      mars2grib-sections-initializers

  SOURCES
      mars2grib-sections-initializers.cc

  NO_AS_NEEDED

  LIBS
      eckit
      metkit
)