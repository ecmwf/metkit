list( APPEND marskit_srcs
BaseProtocol.cc
BaseProtocol.h
Client.cc
Client.h
ClientTask.cc
ClientTask.h
DHSProtocol.cc
DHSProtocol.h
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

if ( HAVE_GRIBPP )
    list( APPEND marskit_srcs
        GribToRequest.cc
        GribToRequest.h )
endif ()

ecbuild_generate_config_headers( DESTINATION ${INSTALL_INCLUDE_DIR}/eckit )

configure_file( marskit_config.h.in marskit_config.h )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/marskit_config.h
         DESTINATION ${INSTALL_INCLUDE_DIR}/marskit )

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
