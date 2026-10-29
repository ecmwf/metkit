# (C) Copyright 2026- ECMWF and individual contributors.
#
# This software is licensed under the terms of the Apache Licence Version 2.0.

# mars2grib corpus ctests, generated from a block file of test cases.
#
# OFF BY DEFAULT. Enable with -DENABLE_MARS2GRIB_ROUNDTRIP_TESTCASES=ON and give the test file, either
#
#     a local copy in tests/mars2grib/testcases/data/ (exactly one *.ekbf; keep it out of git, e.g. with
#       `echo /tests/mars2grib/testcases/data/ >> .git/info/exclude`), or
#     -DMARS2GRIB_TESTCASES_FILE=/path/to/tests.ekbf
#     -DMARS2GRIB_TESTCASES_URL=https://server/path -DMARS2GRIB_TESTCASES_VERSION=v12 [-DMARS2GRIB_TESTCASES_MD5=...]
#       (downloads <URL>/mars2grib-testcases-<VERSION>.ekbf)
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

if( NOT TARGET mars2grib-testcases OR NOT TARGET mars2grib-run-tests-scalar )
    ecbuild_error( "ENABLE_MARS2GRIB_ROUNDTRIP_TESTCASES needs the tools mars2grib-testcases and mars2grib-run-tests-scalar "
                   "(ENABLE_BUILD_GRIB2_TOOLS=ON, eckit with ECKIT_BLOCKFILE)" )
endif()

set( _prefix "mars2grib_corpus" )

set( MARS2GRIB_TESTCASES_FILE    ""       CACHE FILEPATH "Local block file of mars2grib test cases" )
set( MARS2GRIB_TESTCASES_URL     ""       CACHE STRING   "Directory URL holding mars2grib-testcases-<VERSION>.ekbf" )
set( MARS2GRIB_TESTCASES_VERSION ""       CACHE STRING   "Version of the test-case file to download" )
set( MARS2GRIB_TESTCASES_MD5     ""       CACHE STRING   "Expected MD5 of the downloaded file (optional)" )
set( MARS2GRIB_TESTCASES_MODES   "encode;decode" CACHE STRING "Test modes, ;-separated: encode;decode;roundtrip" )
set( MARS2GRIB_TESTCASES_BLOCKS_PER_TEST 1 CACHE STRING
     "Blocks per ctest: 1 = one ctest per block and mode (default), N = chunks of N blocks of the same label combination, 0 = one ctest per label combination" )
set( MARS2GRIB_TESTCASES_TIMEOUT 3600     CACHE STRING   "TIMEOUT of each ctest in seconds (0 = none)" )
set( MARS2GRIB_TESTCASES_ENVIRONMENT ""   CACHE STRING   "Environment of each ctest, ;-separated VAR=VALUE" )

# --- the test-case file ------------------------------------------------------------------------------------

file( GLOB _local_testcases "${CMAKE_CURRENT_SOURCE_DIR}/data/*.ekbf" )
list( LENGTH _local_testcases _local_count )

if( MARS2GRIB_TESTCASES_FILE )
    set( _testcases "${MARS2GRIB_TESTCASES_FILE}" )
    if( NOT EXISTS "${_testcases}" )
        ecbuild_error( "MARS2GRIB_TESTCASES_FILE `${_testcases}` does not exist" )
    endif()
elseif( MARS2GRIB_TESTCASES_URL AND MARS2GRIB_TESTCASES_VERSION )
    set( _name      "mars2grib-testcases-${MARS2GRIB_TESTCASES_VERSION}.ekbf" )
    set( _testcases "${CMAKE_CURRENT_BINARY_DIR}/${_name}" )
    if( NOT EXISTS "${_testcases}" )
        set( _hash )
        if( MARS2GRIB_TESTCASES_MD5 )
            set( _hash EXPECTED_MD5 ${MARS2GRIB_TESTCASES_MD5} )
        endif()
        ecbuild_info( "Downloading ${MARS2GRIB_TESTCASES_URL}/${_name}" )
        file( DOWNLOAD "${MARS2GRIB_TESTCASES_URL}/${_name}" "${_testcases}.part" ${_hash} STATUS _status )
        list( GET _status 0 _code )
        if( NOT _code EQUAL 0 )
            file( REMOVE "${_testcases}.part" )
            ecbuild_error( "Download of ${MARS2GRIB_TESTCASES_URL}/${_name} failed: ${_status}" )
        endif()
        file( RENAME "${_testcases}.part" "${_testcases}" )
    endif()
elseif( _local_count EQUAL 1 )
    set( _testcases "${_local_testcases}" )
elseif( _local_count GREATER 1 )
    ecbuild_error( "Several test files in ${CMAKE_CURRENT_SOURCE_DIR}/data: keep one, or set MARS2GRIB_TESTCASES_FILE" )
else()
    ecbuild_error( "ENABLE_MARS2GRIB_ROUNDTRIP_TESTCASES=ON but no test file: put one *.ekbf in "
                   "${CMAKE_CURRENT_SOURCE_DIR}/data, or set MARS2GRIB_TESTCASES_FILE, or MARS2GRIB_TESTCASES_URL and "
                   "MARS2GRIB_TESTCASES_VERSION" )
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

ecbuild_info( "mars2grib corpus ctests (${_prefix}_*): ${_testcases} (modes: ${_modes}), generated at build time" )
