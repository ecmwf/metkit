# (C) Copyright 2025- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

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
        "METKIT_DIR=${CMAKE_BINARY_DIR}"
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
    cli/test_cli.py
    experimental/integration/test_mars2mars2grib.py
    experimental/mars2grib/test_mars2grib_api.py
    experimental/mars2mars/test_mars2mars_api.py
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
