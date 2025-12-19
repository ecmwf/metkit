ecbuild_add_test(
  TARGET
      mars2grib-concepts-instance

  SOURCES
      mars2grib-concepts-instance.cc

  NO_AS_NEEDED

  LIBS
      eckit
      metkit
)

ecbuild_add_test(
  TARGET
      mars2grib-concepts-builder

  SOURCES
      mars2grib-concepts-builder.cc

  NO_AS_NEEDED

  LIBS
      eckit
      metkit
)

ecbuild_add_test(
  TARGET
      mars2grib-validator

  SOURCES
      mars2grib-validator.cc

  NO_AS_NEEDED

  LIBS
      eckit
      metkit
)
