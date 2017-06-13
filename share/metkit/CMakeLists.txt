list( APPEND files language.json param.json )

foreach( _file ${files} )

    configure_file( ${_file} ${CMAKE_BINARY_DIR}/share/metkit/${_file} @ONLY )

    install( FILES  ${CMAKE_BINARY_DIR}/share/metkit/${_file}
             DESTINATION "share/metkit"
             PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endforeach()

