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
MarsExpandContext.h
MarsExpandContext.cc
MarsParserContext.h
MarsParserContext.cc
MarsParsedRequest.h
MarsParsedRequest.cc
MarsRequestHandle.cc
MarsRequestHandle.h
MetkitTool.h
MetkitTool.cc
RequestEnvironment.cc
RequestEnvironment.h
Parameter.cc
Parameter.h
ParamID.cc
ParamID.h
StepRangeNormalise.h
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

list( APPEND metkit_persistent_srcs
    Param.cc
    Param.h
    Param
    StepRange.cc
    StepRange.h
    StepRange
)
list( APPEND metkit_srcs ${metkit_persistent_srcs} )


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
        grib/GribIterator.h
        grib/GribIterator.cc
        grib/GribDataBlob.h
        grib/GribDataBlob.cc
        grib/GribMetaData.h
        grib/GribMetaData.cc
        grib/LibEccodes.h
        grib/LibEccodes.cc
	pointdb/DataSource.cc
	pointdb/DataSource.h
	pointdb/FieldIndexer.cc
	pointdb/FieldIndexer.h
	pointdb/GribDataSource.cc
	pointdb/GribDataSource.h
	pointdb/GribFieldInfo.cc
	pointdb/GribFieldInfo.h
	pointdb/GribHandleDataSource.cc
	pointdb/GribHandleDataSource.h
	pointdb/PointIndex.cc
	pointdb/PointIndex.h
	pointdb/bits.h
	pointdb/masks.h
        )

    set( grib_libs eccodes )

endif ()

if ( HAVE_BUFR )

    list( APPEND metkit_srcs
        bufr/BufrToRequest.cc
        bufr/BufrToRequest.h
        bufr/BufrHandle.h
        bufr/BufrHandle.cc
        )

    set( bufr_libs eccodes )

endif ()


ecbuild_add_library(

    TARGET metkit

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/metkit

    PERSISTENT
        ${metkit_persistent_srcs}

    GENERATED
        metkit_version.cc

    SOURCES
        ${metkit_srcs}

    PRIVATE_INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${ECCODES_INCLUDE_DIRS}
        ${NETCDF_INCLUDE_DIRS}
        ${ODC_INCLUDE_DIRS}

    LIBS
        eckit eckit_option
        ${odc_libs}
        ${grib_libs}
        ${bufr_libs}
        ${netcdflibs}
)

ecbuild_add_library(

        TARGET metkit_odb

        INSTALL_HEADERS LISTED

        CONDITION HAVE_ODB2

        HEADER_DESTINATION
            ${INSTALL_INCLUDE_DIR}/metkit

        SOURCES
          odb/IdMapper.cc
          odb/IdMapper.h
          odb/OdbToRequest.cc
          odb/OdbToRequest.h

        PRIVATE_INCLUDES
            ${ECKIT_INCLUDE_DIRS}
            ${ODC_INCLUDE_DIRS}

        LIBS
            metkit
            odccore
)
