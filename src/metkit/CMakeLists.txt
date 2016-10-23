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
MarsExpension.cc
MarsExpension.h
MarsLanguage.cc
MarsLanguage.h
MarsParser.cc
MarsParser.h
MarsRequestHandle.cc
MarsRequestHandle.h
RequestEnvironment.cc
RequestEnvironment.h
config/LibMetkit.cc
config/LibMetkit.h
types/Type.cc
types/Type.h
types/TypeDate.cc
types/TypeDate.h
types/TypeAny.cc
types/TypeAny.h
types/TypeExpver.cc
types/TypeExpver.h
types/TypeToByList.cc
types/TypeToByList.h
types/TypeEnum.cc
types/TypeEnum.h
types/TypePostProc.cc
types/TypePostProc.h
types/TypeInteger.cc
types/TypeInteger.h
types/TypeFloat.cc
types/TypeFloat.h
types/TypeTime.cc
types/TypeTime.h
types/TypesFactory.cc
types/TypesFactory.h
)

if ( HAVE_GRIB )

    list( APPEND metkit_srcs
        grib/GribToRequest.cc
        grib/GribToRequest.h
        grib/EmosFile.h
        grib/EmosFile.cc
        grib/GribFile.h
        grib/GribFile.cc
        grib/GribAccessor.h
        grib/GribAccessor.cc
        grib/GribMutator.h
        grib/GribMutator.cc
        grib/GribIndex.h
        grib/GribIndex.cc
        grib/GribHandle.h
        grib/GribHandle.cc
        grib/GribDataBlob.h
        grib/GribDataBlob.cc
        grib/GribMetaData.h
        grib/GribMetaData.cc
        )

        set( grib_handling_pkg grib_api )
        if( HAVE_ECCODES )
          set( grib_handling_pkg eccodes )
        endif()

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
        eckit ${grib_handling_pkg} )