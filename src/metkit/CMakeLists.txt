list( APPEND metkit_srcs
BaseProtocol.cc
BaseProtocol.h
ClientTask.cc
ClientTask.h
DHSProtocol.cc
DHSProtocol.h
MarsHandle.cc
MarsHandle.h
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
  list( APPEND metkit_srcs
        GribToRequest.cc
        GribToRequest.h )
  set( GRIBPP_LIB gribpp )
endif ()

ecbuild_generate_config_headers( DESTINATION ${INSTALL_INCLUDE_DIR}/metkit )

configure_file( metkit_config.h.in metkit_config.h )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/metkit_config.h
         DESTINATION ${INSTALL_INCLUDE_DIR}/metkit )

ecbuild_add_library(

    TARGET metkit

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/metkit

    SOURCES
        ${metkit_srcs}

    PRIVATE_INCLUDES
        ${GRIBPP_INCLUDE_DIRS}
        ${ECKIT_INCLUDE_DIRS}

    LIBS
        eckit ${GRIBPP_LIB} )
