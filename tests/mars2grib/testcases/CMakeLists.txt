# (C) Copyright 2026- ECMWF and individual contributors.
#
# This software is licensed under the terms of the Apache Licence Version 2.0.

# mars2grib corpus ctests, generated from a block file of test cases.
#
# OFF BY DEFAULT. Enable with -DENABLE_MARS2GRIB_ROUNDTRIP_TESTCASES=ON.
#
# The test file is always downloaded (once, into the build directory) from the ECMWF sites repository:
#     <URL>/mars2grib-testcases-<VERSION>.ekbf
# with the defaults below; versions named after the file's MD5 are checked on download. To use another
# published file: -DMARS2GRIB_TESTCASES_VERSION=<md5> (and -DMARS2GRIB_TESTCASES_URL / _MD5 if needed).
#
# The ctests are created AFTER the build: the target `mars2grib_testcases_ctests` (part of `all`) runs
# `mars2grib-testcases ctest-list`, which reads the block table and writes, by default, two ctests per
# block: one in "encode" mode (mars/misc/opt -> set-list compared with `out`) and one in "decode" mode
# (`out` -> GRIB -> grib2mars compared with mars/misc). Tests are named mars2grib_corpus_<mode>_b<block>_<labels>
# and labelled with the mode, the block id and every block label. CTest includes the generated file
# when it runs, so the tests always match the test file. Then:
#
#     ctest -j 32 -L mars2grib_corpus           # everything
#     ctest -j 32 -L stream=enfo                # one stream
#     ctest -j 32 -L mars2grib_corpus-decode    # one mode
#     ctest -L "^block=42$"                    # one block (both modes)
#     ctest -N -L mars2grib_corpus              # list the tests (after building)
#
# Standard scripts generated in the build tree (tests/mars2grib/testcases/), without arguments:
#     ./rerun-all-tests.sh        all tests with mars2grib-run-tests; self-contained failure reports in failure-reports/
#     ./regenerate-all-tests.sh   regenerate the test file with the current code into regenerated/
#     ./upload-test-file.sh       upload the regenerated file to Nexus (prompts for credentials)

if( NOT TARGET mars2grib-testcases OR NOT TARGET mars2grib-run-tests-scalar )
    ecbuild_error( "ENABLE_MARS2GRIB_ROUNDTRIP_TESTCASES needs the tools mars2grib-testcases and mars2grib-run-tests-scalar "
                   "(ENABLE_BUILD_GRIB2_TOOLS=ON, eckit with ECKIT_BLOCKFILE)" )
endif()

set( _prefix "mars2grib_corpus" )

set( MARS2GRIB_TESTCASES_URL     ""       CACHE STRING
     "Directory URL holding mars2grib-testcases-<VERSION>.ekbf (empty = ECMWF sites repository)" )
set( MARS2GRIB_TESTCASES_VERSION ""       CACHE STRING
     "Version of the test-case file to download (empty = the default file; an MD5 version is also checked)" )
set( MARS2GRIB_TESTCASES_MD5     ""       CACHE STRING   "Expected MD5 of the downloaded file (optional)" )

# Built-in defaults, used when the cache variables above are empty.
set( _default_url     "https://sites.ecmwf.int/repository/metkit/test-data/mars2grib" )
set( _default_version "910e7d20b76f19b1eef5f4edb16c25c5" )
set( MARS2GRIB_TESTCASES_MODES   "encode;decode" CACHE STRING "Test modes, ;-separated: encode;decode;roundtrip" )
set( MARS2GRIB_TESTCASES_BLOCKS_PER_TEST 1 CACHE STRING
     "Blocks per ctest: 1 = one ctest per block and mode (default), N = chunks of N blocks of the same label combination, 0 = one ctest per label combination" )
set( MARS2GRIB_TESTCASES_TIMEOUT 3600     CACHE STRING   "TIMEOUT of each ctest in seconds (0 = none)" )
set( MARS2GRIB_TESTCASES_ENVIRONMENT ""   CACHE STRING   "Environment of each ctest, ;-separated VAR=VALUE" )

# --- the test-case file ------------------------------------------------------------------------------------

set( _url     "${MARS2GRIB_TESTCASES_URL}" )
set( _version "${MARS2GRIB_TESTCASES_VERSION}" )
set( _md5     "${MARS2GRIB_TESTCASES_MD5}" )
if( NOT _url )
    set( _url "${_default_url}" )
endif()
if( NOT _version )
    set( _version "${_default_version}" )
endif()
string( LENGTH "${_version}" _version_length )
if( NOT _md5 AND _version_length EQUAL 32 AND _version MATCHES "^[0-9a-f]+$" )
    set( _md5 "${_version}" )  # versions named after their MD5 are checked automatically
endif()

set( _name      "mars2grib-testcases-${_version}.ekbf" )
set( _testcases "${CMAKE_CURRENT_BINARY_DIR}/${_name}" )
if( NOT EXISTS "${_testcases}" )
    set( _hash )
    if( _md5 )
        set( _hash EXPECTED_MD5 ${_md5} )
    endif()
    ecbuild_info( "Downloading ${_url}/${_name}" )
    file( DOWNLOAD "${_url}/${_name}" "${_testcases}.part" ${_hash} STATUS _status TLS_VERIFY ON )
    list( GET _status 0 _code )
    if( NOT _code EQUAL 0 )
        file( REMOVE "${_testcases}.part" )
        ecbuild_error( "Download of ${_url}/${_name} failed: ${_status}" )
    endif()
    file( RENAME "${_testcases}.part" "${_testcases}" )
endif()

# --- generate the ctests after the build -------------------------------------------------------------------

set( _generated "${CMAKE_CURRENT_BINARY_DIR}/mars2grib_testcases_ctests.cmake" )
string( REPLACE ";" "," _modes       "${MARS2GRIB_TESTCASES_MODES}" )
string( REPLACE ";" "," _environment "${MARS2GRIB_TESTCASES_ENVIRONMENT}" )

set( _environment_args )
if( _environment )
    set( _environment_args --environment "${_environment}" )
endif()

add_custom_command(
    OUTPUT  "${_generated}"
    COMMAND $<TARGET_FILE:mars2grib-testcases> ctest-list "${_testcases}"
            --runner          $<TARGET_FILE:mars2grib-run-tests-scalar>
            --prefix          ${_prefix}
            --modes           "${_modes}"
            --blocks-per-test ${MARS2GRIB_TESTCASES_BLOCKS_PER_TEST}
            --timeout         ${MARS2GRIB_TESTCASES_TIMEOUT}
            --reports-dir     "${CMAKE_CURRENT_BINARY_DIR}/failure-reports"
            --output          "${_generated}"
            ${_environment_args}
    DEPENDS mars2grib-testcases mars2grib-run-tests-scalar "${_testcases}"
    COMMENT "Generating mars2grib test-case ctests from ${_testcases}"
    VERBATIM )

add_custom_target( mars2grib_testcases_ctests ALL DEPENDS "${_generated}" )

# CTest includes this at test time: it pulls in the generated tests, or defines one failing test
# explaining what to do if they have not been generated yet.
configure_file( include_ctests.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/include_ctests.cmake" @ONLY )
set_property( DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${CMAKE_CURRENT_BINARY_DIR}/include_ctests.cmake" )

# --- standard scripts, in the build tree, without arguments ----------------------------------------------
#   rerun-all-tests.sh       mars2grib-run-tests (encode, decode) with failure reports in failure-reports/
#   regenerate-all-tests.sh  mars2grib-regenerate-tests into regenerated/mars2grib-testcases-<md5>.ekbf
#   upload-test-file.sh      upload that file to Nexus (asks for username/password) and check it

set( _regen_keys              "class,stream,type,levtype,timeShape" )
set( _regen_records_per_block 100 )
set( _regen_zstd_level        22 )
set( _nexus_remote_path       "metkit/mars2grib" )
set( _nexus_download_url      "https://get.ecmwf.int/repository/test-data/metkit/mars2grib" )

foreach( _script rerun-all-tests regenerate-all-tests upload-test-file )
    configure_file( ${_script}.sh.in "${CMAKE_CURRENT_BINARY_DIR}/${_script}.sh.configured" @ONLY )
    if( CMAKE_VERSION VERSION_LESS 3.19 )
        file( GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_script}.sh"
                       INPUT  "${CMAKE_CURRENT_BINARY_DIR}/${_script}.sh.configured" )
    else()
        file( GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_script}.sh"
                       INPUT  "${CMAKE_CURRENT_BINARY_DIR}/${_script}.sh.configured"
                       FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
                                        WORLD_READ WORLD_EXECUTE )
    endif()
endforeach()

ecbuild_info( "mars2grib corpus ctests (${_prefix}_*): ${_testcases} (modes: ${_modes}), generated at build time" )
