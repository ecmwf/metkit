list( APPEND marskit_srcs
    MarsRequest.h
    MarsRequest.cc
    GribToRequest.h
    GribToRequest.cc
)

ecbuild_add_library(

    TARGET marskit

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/marskit

    SOURCES
        ${marskit_srcs}

    PRIVATE_INCLUDES
        ${OOGRIB_INCLUDE_DIRS}
        ${ECKIT_INCLUDE_DIRS}

    LIBS
        oogrib eckit )
