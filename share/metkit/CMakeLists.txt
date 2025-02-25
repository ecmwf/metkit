list( APPEND files language.yaml paramids.yaml params.yaml params-static.yaml param-matching.yaml bufr-subtypes.yaml axis.yaml shortname-context.yaml )

foreach( _file ${files} )

    configure_file( ${_file} ${CMAKE_BINARY_DIR}/share/metkit/${_file} COPYONLY )

    if( HAVE_METKIT_CONFIG )
        install( FILES  ${CMAKE_BINARY_DIR}/share/metkit/${_file}
                DESTINATION "share/metkit"
                PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
    endif()

endforeach()

add_subdirectory( odb )
