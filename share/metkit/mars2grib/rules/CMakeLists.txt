list( APPEND files_mars2grib_rules MapSection2.yaml CategoriesToPDT.yaml TimeDescription.yaml LevTypeToTypeOfLevel.yaml MapLevel.yaml )

foreach( _file ${files_mars2grib_rules} )

    configure_file( ${_file} ${CMAKE_BINARY_DIR}/share/metkit/mars2grib/rules/${_file} COPYONLY )

    install( FILES  ${CMAKE_BINARY_DIR}/share/metkit/mars2grib/rules/${_file}
             DESTINATION "share/metkit/mars2grib/rules"
             PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endforeach()

