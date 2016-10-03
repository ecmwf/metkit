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

if ( HAVE_GRIB )
  list( APPEND metkit_srcs
        GribToRequest.cc
        GribToRequest.h )
  set( GRIB_LIB grib )
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
        ${ECKIT_INCLUDE_DIRS}
        ${GRIB_API_INCLUDE_DIRS}

    LIBS
        eckit ${GRIB_LIB} )
