### provides MARS requests from GRIB files

ecbuild_add_executable( TARGET   grib-to-mars-request

    CONDITION HAVE_GRIB

    SOURCES  grib-to-mars-request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}

    LIBS     metkit
)

ecbuild_add_executable( TARGET   grib-blob

    CONDITION HAVE_GRIB

    SOURCES  grib-blob.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}

    LIBS     metkit eckit_option eckit
)

ecbuild_add_executable( TARGET read-bufr
  CONDITION HAVE_BUFR
  SOURCES  read-bufr.cc
  INCLUDES ${ECCODES_INCLUDE_DIRS} ${ECKIT_INCLUDE_DIRS}
  LIBS     metkit eckit_option eckit eccodes
)

ecbuild_add_executable(

    TARGET   parse-mars-request

    # CONDITION HAVE_GRIB

    SOURCES  parse-mars-request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}

    LIBS     metkit
)

# ecbuild_add_executable( TARGET     ncmerge
#                         SOURCES    ncmerge.cc
#                         CONDITION  HAVE_NETCDF
#                         INCLUDES   ${NETCDF_INCLUDE_DIRS}
#                         LIBS       metkit ${NETCDF_LIBRARIES} )


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/nccompare.py
  DESTINATION ${CMAKE_BINARY_DIR}/bin
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

