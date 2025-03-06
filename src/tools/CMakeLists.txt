### provides MARS requests from GRIB files

# ecbuild_add_executable( TARGET   grib-to-mars-request

#     CONDITION HAVE_GRIB

#     SOURCES  grib-to-mars-request.cc

#     INCLUDES
#         ${ECKIT_INCLUDE_DIRS}

#     LIBS     metkit
# )

# ecbuild_add_executable( TARGET   grib-blob

#     CONDITION HAVE_GRIB

#     SOURCES  grib-blob.cc

#     INCLUDES
#         ${ECKIT_INCLUDE_DIRS}

#     LIBS     metkit eckit_option eckit
# )


# ecbuild_add_executable( TARGET   bufr-to-mars-request

#     CONDITION HAVE_BUFR

#     SOURCES  bufr-to-mars-request.cc

#     INCLUDES ${ECCODES_INCLUDE_DIRS} ${ECKIT_INCLUDE_DIRS}

#     LIBS     metkit eckit_option eckit eccodes
# )

ecbuild_add_executable( TARGET    parse-mars-request
    CONDITION HAVE_GRIB AND HAVE_METKIT_BUILD_TOOLS
    SOURCES   parse-mars-request.cc
    INCLUDES  ${ECKIT_INCLUDE_DIRS}
    LIBS        metkit eckit_option eckit eccodes
    NO_AS_NEEDED
)

ecbuild_add_executable( TARGET    odb-to-request
    SOURCES   odb-to-request.cc
    CONDITION HAVE_ODB AND HAVE_METKIT_BUILD_TOOLS
    INCLUDES  ${ECKIT_INCLUDE_DIRS}
    NO_AS_NEEDED
    LIBS      metkit eckit_option )

ecbuild_add_executable( TARGET    bufr-sanity-check
    SOURCES   bufr-sanity-check.cc
    CONDITION HAVE_BUFR AND HAVE_METKIT_BUILD_TOOLS
    INCLUDES  ${ECKIT_INCLUDE_DIRS}
    NO_AS_NEEDED
    LIBS      metkit eckit_option )

ecbuild_add_executable(
    TARGET        mars-archive-script
    SOURCES       mars-archive-script.cc
    CONDITION     HAVE_METKIT_BUILD_TOOLS
    INCLUDES      ${ECKIT_INCLUDE_DIRS}
    NO_AS_NEEDED
    LIBS          metkit
)

# ecbuild_add_executable( TARGET     ncmerge
#                         SOURCES    ncmerge.cc
#                         CONDITION  HAVE_NETCDF
#                         LIBS       metkit NetCDF::NetCDF_C )


if(HAVE_BUILD_TOOLS)
    list( APPEND tools nccompare.py compare-mars-requests.py )

    foreach( _tool ${tools} )

        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${_tool}
            DESTINATION ${CMAKE_BINARY_DIR}/bin
            FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

    endforeach()

    install( FILES  ${CMAKE_BINARY_DIR}/bin/compare-mars-requests.py
        DESTINATION "bin"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ )

endif()

