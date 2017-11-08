### config headers

ecbuild_generate_config_headers( DESTINATION ${INSTALL_INCLUDE_DIR}/metkit )

configure_file( metkit_config.h.in    metkit_config.h )
configure_file( metkit_version.h.in   metkit_version.h )
configure_file( metkit_version.cc.in  metkit_version.cc )

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/metkit_config.h
            ${CMAKE_CURRENT_BINARY_DIR}/metkit_version.h
        DESTINATION
            ${INSTALL_INCLUDE_DIR}/metkit )

### metkit sources

list( APPEND metkit_srcs
metkit_version.cc
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
Parameter.cc
Parameter.h
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
types/TypeRegex.cc
types/TypeRegex.h
types/TypeParam.cc
types/TypeParam.h
types/TypeInteger.cc
types/TypeInteger.h
types/TypeRange.cc
types/TypeRange.h
types/TypeFloat.cc
types/TypeFloat.h
types/TypeTime.cc
types/TypeTime.h
types/TypeDate.cc
types/TypeDate.h
types/TypeMixed.cc
types/TypeMixed.h
types/TypesFactory.cc
types/TypesFactory.h
)

if ( HAVE_GRIB )

    list( APPEND metkit_srcs
        grib/GribToRequest.cc
        grib/GribToRequest.h
        grib/MetFile.h
        grib/MetFile.cc
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

    set( grib_handling_pkg     eccodes )

else()
    set( grib_handling_pkg )
endif ()

ecbuild_add_library(

    TARGET metkit

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/metkit

    GENERATED
        metkit_version.cc

    SOURCES
        ${metkit_srcs}

    PRIVATE_INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${ECCODES_INCLUDE_DIRS}
        ${NETCDF_INCLUDE_DIRS}

    LIBS
        eckit
        ${grib_handling_pkg}
        ${netcdflibs}
)
