list( APPEND mars_tools_bins marsgen grib2request bufr2request check_fs obscmp prearcbufr ecmars)

foreach( tool ${mars_tools_bins} )

    # here we use the fact that each tool has only one C file that matches its name
    ecbuild_add_executable( TARGET     ${tool}
                            SOURCES    ${tool}.c
                            INCLUDES  ${GRIB_API_INCLUDE_DIRS}
                            LIBS       mars )

endforeach()

# this executable only makes sense under the previous FDB 4.x series

ecbuild_add_executable( TARGET     grib2fdb
                        SOURCES    grib2fdb.c
                        CONDITION  HAVE_FDB AND fdb_VERSION VERSION_LESS 5.0
                        INCLUDES  ${GRIB_API_INCLUDE_DIRS}
                        LIBS       mars )

ecbuild_add_executable( TARGET     makedb_bufr
                        SOURCES    makedb_bufr.c
                        CONDITION  HAVE_FDB AND fdb_VERSION VERSION_LESS 5.0
                        INCLUDES  ${GRIB_API_INCLUDE_DIRS}
                        LIBS       mars )

ecbuild_add_executable( TARGET     filterbufr
                        SOURCES    filterbufr.c
                        CONDITION  LIBEMOS_HAVE_LIBEMOS_BUFRDC
                        INCLUDES  ${GRIB_API_INCLUDE_DIRS}
                        LIBS       mars )

ecbuild_add_executable( TARGET     ncmerge
						SOURCES    ncmerge.cc Attribute.cc Attribute.h Dimension.cc Dimension.h
						Exceptions.h Exceptions.cc Endowed.cc Endowed.h InputField.cc InputField.h
                        OutputField.cc OutputField.h Matrix.h Matrix.cc MergeDataMatrix.h MergeDataMatrix.cc
                        MergeCoordinateMatrix.h MergeCoordinateMatrix.cc
                        InputMatrix.h InputMatrix.cc
			            Counted.h Counted.cc
                        DummyMatrix.h DummyMatrix.cc
                        DummyInputVariable.h DummyInputVariable.cc
                        Reshape.h Reshape.cc
                        InputAttribute.cc InputAttribute.h
                        InputDimension.cc InputDimension.h InputVariable.cc InputVariable.h
			            VirtualInputDimension.h VirtualInputDimension.cc
			            VirtualOutputDimension.h VirtualOutputDimension.cc
                        OutputAttribute.cc OutputAttribute.h OutputDimension.cc OutputDimension.h OutputVariable.cc OutputVariable.h
                        DataInputVariable.cc DataInputVariable.h
                        DataOutputVariable.cc DataOutputVariable.h
                        Remapping.h Remapping.cc
                        SimpleInputVariable.cc
                        SimpleInputVariable.h
                        SimpleOutputVariable.cc
                        SimpleOutputVariable.h
                        CellMethodInputVariable.cc
                        CellMethodInputVariable.h
                        CellMethodOutputVariable.cc
                        CellMethodOutputVariable.h
                        Codec.cc Codec.h Mapper.h
                        GregorianDateCodec.h
                        GregorianDateCodec.cc
                        Tokenizer.cc Tokenizer.h Date.cc Time.cc Time.h Date.h DateTime.cc DateTime.h
                        Translator.h Translator.cc Seconds.cc Seconds.h
                        NCFile.h NCFile.cc NCFileCache.h NCFileCache.cc
                        CoordinateOutputVariable.cc CoordinateOutputVariable.h
                        ScalarCoordinateOutputVariable.cc ScalarCoordinateOutputVariable.h
                        CoordinateInputVariable.h CoordinateInputVariable.cc
                        ScalarCoordinateInputVariable.h ScalarCoordinateInputVariable.cc
                        Step.h Step.cc
                        MergePlan.h MergePlan.cc MergeDataStep.h MergeDataStep.cc
                        MergeCoordinateStep.h MergeCoordinateStep.cc
                        UpdateCoordinateStep.h UpdateCoordinateStep.cc
                        ReshapeVariableStep.cc ReshapeVariableStep.h HyperCube.cc HyperCube.h
                        Field.cc Field.h Type.cc Type.h Value.cc Value.h Variable.cc Variable.h ncmerge.cc
						CONDITION  HAVE_NETCDF
                        INCLUDES   ${NETCDF_INCLUDE_DIRS}
						LIBS       ${NETCDF_LIBRARIES} )

ecbuild_add_executable( TARGET     nctest
                        SOURCES    nctest.c
                        CONDITION  HAVE_NETCDF
                        INCLUDES   ${NETCDF_INCLUDE_DIRS}
                        LIBS       ${NETCDF_LIBRARIES} )


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/nccompare.py
  DESTINATION ${CMAKE_BINARY_DIR}/bin
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

# two_dimensions_merge
list(APPEND merge_tests_ok simple_two_way_merge
        simple_three_way_merge
        complex_two_way_merge
        type_upgrade_merge
        attribute_merge
        multi_merge
        multi_dimensions_merge
        signature_merge_1
        signature_merge_2
        new_missing_values_merge
        new_dimension_merge_1
        new_dimension_merge_2
        retype_merge
        coordinates_overlap_merge_1
        coordinates_overlap_merge_2
        coordinates_overlap_merge_3
        coordinates_overlap_merge_4
        coordinates_overlap_merge_5
        coordinates_overlap_merge_6
        date_merge_1
        date_merge_2
        variable_merge_1
        variable_merge_2
        variable_merge_3
        variable_merge_4
        variable_merge_5
        bounds_merge_1
        bounds_merge_2
        monotonic
        )

list(APPEND merge_tests_fail bounds_merge)
list(APPEND merge_tests_only coordinates_overlap_merge)

foreach(_TEST
    bounds_merge_2
    # ${merge_tests_ok}
    )
    ecbuild_add_test(TARGET "nccombine_${_TEST}"
                     COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${_TEST}.sh")
endforeach()

