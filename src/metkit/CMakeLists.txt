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
mars/BaseProtocol.cc
mars/BaseProtocol.h
mars/ClientTask.cc
mars/ClientTask.h
mars/DHSProtocol.cc
mars/DHSProtocol.h
mars/MarsHandle.cc
mars/MarsHandle.h
mars/MarsLocation.cc
mars/MarsLocation.h
mars/MarsRequest.cc
mars/MarsRequest.h
mars/MarsExpension.cc
mars/MarsExpension.h
mars/MarsLanguage.cc
mars/MarsLanguage.h
mars/MarsParser.cc
mars/MarsParser.h
mars/MarsExpandContext.h
mars/MarsExpandContext.cc
mars/MarsParserContext.h
mars/MarsParserContext.cc
mars/MarsParsedRequest.h
mars/MarsParsedRequest.cc
mars/MarsRequestHandle.cc
mars/MarsRequestHandle.h
tool/MetkitTool.h
tool/MetkitTool.cc
mars/RequestEnvironment.cc
mars/RequestEnvironment.h
mars/Parameter.cc
mars/Parameter.h
mars/ParamID.cc
mars/ParamID.h
mars/StepRangeNormalise.h
config/LibMetkit.cc
config/LibMetkit.h
mars/Type.cc
mars/Type.h
mars/TypeDate.cc
mars/TypeDate.h
mars/TypeAny.cc
mars/TypeAny.h
mars/TypeExpver.cc
mars/TypeExpver.h
mars/TypeToByList.cc
mars/TypeToByList.h
mars/TypeEnum.cc
mars/TypeEnum.h
mars/TypeRegex.cc
mars/TypeRegex.h
mars/TypeParam.cc
mars/TypeParam.h
mars/TypeInteger.cc
mars/TypeInteger.h
mars/TypeRange.cc
mars/TypeRange.h
mars/TypeFloat.cc
mars/TypeFloat.h
mars/TypeTime.cc
mars/TypeTime.h
mars/TypeDate.cc
mars/TypeDate.h
mars/TypeMixed.cc
mars/TypeMixed.h
mars/TypesFactory.cc
mars/TypesFactory.h
)

list( APPEND metkit_persistent_srcs
    mars/Param.cc
    mars/Param.h
    mars/Param
    mars/StepRange.cc
    mars/StepRange.h
    mars/StepRange
)
list( APPEND metkit_srcs ${metkit_persistent_srcs} )


if ( HAVE_GRIB )

    list( APPEND metkit_srcs
        grib/GribToRequest.cc
        grib/GribToRequest.h
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
        codes/Message.h
        codes/Message.cc
        codes/Reader.h
        codes/Reader.cc
        codes/LibEccodes.h
        codes/LibEccodes.cc
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
