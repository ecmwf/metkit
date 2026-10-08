list( APPEND files language.yaml modifiers.yaml reportype.yaml obstype.yaml paramids.yaml params.yaml params-static.yaml param-matching.yaml chemids.yaml bufr-subtypes.yaml axis.yaml shortname-context.yaml )

foreach( _file ${files} )

    configure_file( ${_file} ${CMAKE_BINARY_DIR}/share/metkit/${_file} COPYONLY )

    list( APPEND installed_files ${CMAKE_BINARY_DIR}/share/metkit/${_file} )

    if( HAVE_METKIT_CONFIG )
        install( FILES  ${CMAKE_BINARY_DIR}/share/metkit/${_file}
                DESTINATION "share/metkit"
                PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
    endif()

endforeach()

if( HAVE_METKIT_CONFIG )

    # re-run whenever a config yaml is copied or the generator tool is rebuilt;
    # force regeneration since create-binary-files skips writing if the file already exists;
    # disable plugin auto-loading, since sibling projects' libraries may not be built yet at this point
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/share/metkit/params.bin
                    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/share/metkit/params.bin
                    COMMAND ${CMAKE_COMMAND} -E env METKIT_HOME=${CMAKE_BINARY_DIR} METKIT_FORCE_BINFILE_CREATION=1 AUTO_LOAD_PLUGINS=0 $<TARGET_FILE:create-binary-files>
                    DEPENDS create-binary-files ${installed_files}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Create binary files for Metkit configuration")

    add_custom_target( metkit_params_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/share/metkit/params.bin )

    install( FILES ${CMAKE_BINARY_DIR}/share/metkit/params.bin
            DESTINATION "share/metkit"
            PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
endif()


add_subdirectory( odb )
