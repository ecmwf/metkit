# SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

# Create the pytest-tmp folder for encompassing the different scenarios
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pytest-tmp")

function(add_pymetkit_test scenario test)
    get_filename_component(stem ${test} NAME_WLE)
    add_test(
        NAME ${scenario}_${stem}
        COMMAND ${Python_EXECUTABLE}
            -m pytest
            --basetemp ${CMAKE_CURRENT_BINARY_DIR}/pytest-tmp/${scenario}/
            -vv -s ${CMAKE_CURRENT_SOURCE_DIR}/${test}
    )
    set(_env
        "PYTHONPATH=${CMAKE_BINARY_DIR}/pymetkit-python-package-staging:$ENV{PYTHONPATH}"
        # prevent findlibs in pymetkit from picking up any globally installed metkit
        "METKIT_DIR=$<TARGET_FILE_DIR:metkit>/.."
        "FINDLIBS_DISABLE_PACKAGE=yes"
        # metkit_env provides METKIT_HOME and ECCODES_DEFINITION_PATH for expansion
        ${metkit_env}
    )
    set_tests_properties(${scenario}_${stem}
        PROPERTIES
        RESOURCE_LOCK ${scenario}
        ENVIRONMENT "${_env}"
        LABELS ${scenario}
    )
endfunction()

set(test_files
    integration/mars_request/test_construction.py
    integration/mars_request/test_accessors.py
    integration/mars_request/test_selection.py
    integration/mars_request/test_expand.py
    integration/mars_request/test_merge.py
    integration/mars_request/test_equality.py
    integration/mars_request/test_parse.py
    integration/test_paramdb.py
    cli/test_cli.py
)

foreach(test_file ${test_files})
    add_pymetkit_test("pymetkit" ${test_file})
endforeach()

set(doc_test_files
    ../../docs/pymetkit/examples.rst
)

foreach(test_file ${doc_test_files})
    add_pymetkit_test("pymetkit_doc" ${test_file})
endforeach()


if( HAVE_MARS2MARS AND HAVE_PYTHON_METKIT_INTERFACE )
    set(test_files_mars2mars
        experimental/mars2mars/test_mars2mars_api.py
    )
    foreach(test_file ${test_files_mars2mars})
        add_pymetkit_test("pymetkit_experimental_mars2mars" ${test_file})
    endforeach()
endif()


if( HAVE_MARS2GRIB AND HAVE_PYTHON_METKIT_INTERFACE )
    set(test_files_mars2grib
        experimental/mars2grib/test_mars2grib_api.py
    )
    foreach(test_file ${test_files_mars2grib})
        add_pymetkit_test("pymetkit_experimental_mars2grib" ${test_file})
    endforeach()
endif()


if( HAVE_MARS2MARS AND HAVE_MARS2GRIB AND HAVE_PYTHON_METKIT_INTERFACE )
    set(test_files_mars2mars2grib
        experimental/integration/test_mars2mars2grib.py
    )

    foreach(test_file ${test_files_mars2mars2grib})
        add_pymetkit_test("pymetkit_experimental_mars2mars2grib" ${test_file})
    endforeach()

endif()

