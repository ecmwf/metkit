list( APPEND marskit_srcs
BaseProtocol.cc
BaseProtocol.h
Client.cc
Client.h
ClientTask.cc
ClientTask.h
DHSProtocol.cc
DHSProtocol.h
GribToRequest.cc
GribToRequest.h
MarsHandle.cc
MarsHandle.h
MarsHandleFactory.cc
MarsHandleFactory.h
MarsLocation.cc
MarsLocation.h
MarsRequest.cc
MarsRequest.h
MarsRequestHandle.cc
MarsRequestHandle.h
RequestEnvironment.cc
RequestEnvironment.h
)

ecbuild_add_library(

    TARGET marskit

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/marskit

    SOURCES
        ${marskit_srcs}

    PRIVATE_INCLUDES
        ${GRIBPP_INCLUDE_DIRS}
        ${ECKIT_INCLUDE_DIRS}

    LIBS
        gribpp eckit_ecml eckit )
