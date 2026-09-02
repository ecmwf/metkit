find_package(Doxygen 1.9 REQUIRED)
find_package(Sphinx REQUIRED)

# --- Paths ---
# The public API surface Doxygen parses: the C API header plus the main
# high-level C++ entry point (the MarsRequest class), which live under
# src/metkit/api/ and src/metkit/mars/ respectively.
set(METKIT_API_HEADERS
    "${PROJECT_SOURCE_DIR}/src/metkit/api/metkit_c.h"
    "${PROJECT_SOURCE_DIR}/src/metkit/mars/MarsRequest.h")
set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(SPHINX_OUTPUT_DIR  "${CMAKE_CURRENT_BINARY_DIR}/sphinx")
set(DOXYGEN_XML_DIR    "${DOXYGEN_OUTPUT_DIR}/xml")
set(DOXYGEN_HTML_DIR   "${DOXYGEN_OUTPUT_DIR}/html")
string(JOIN " " DOXYGEN_INPUT_DIR ${METKIT_API_HEADERS})
set(METKIT_VERSION   "${PROJECT_VERSION}")

# Staging dir whose "doxygen/" subfolder holds the generated Doxygen HTML.
# Sphinx copies it verbatim into the site (see conf.py / html_extra_path).
set(SPHINX_EXTRA_DIR   "${CMAKE_CURRENT_BINARY_DIR}/extra")

# --- Input file globs ---
set(METKIT_PUBLIC_HEADERS ${METKIT_API_HEADERS})
file(GLOB_RECURSE PYMETKIT_SRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/pymetkit/*.py")
file(GLOB_RECURSE SPHINX_INPUT_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.rst")
list(FILTER SPHINX_INPUT_FILES EXCLUDE REGEX "_build")

# --- Output stamps ---
set(DOXYGEN_INDEX_FILE "${DOXYGEN_OUTPUT_DIR}/html/index.html")
set(SPHINX_INDEX_FILE  "${SPHINX_OUTPUT_DIR}/index.html")

# --- Configure Doxyfile ---
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    @ONLY
)

# --- Doxygen ---
add_custom_command(
    OUTPUT  ${DOXYGEN_INDEX_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIR}"
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${SPHINX_EXTRA_DIR}/doxygen"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${DOXYGEN_HTML_DIR}" "${SPHINX_EXTRA_DIR}/doxygen"
    DEPENDS ${METKIT_PUBLIC_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Running Doxygen on Metkit API headers"
)

# --- Sphinx ---
add_custom_command(
    OUTPUT  ${SPHINX_INDEX_FILE}
    COMMAND
        ${CMAKE_COMMAND} -E env
        "DOXYGEN_HTML_EXTRA_DIR=${SPHINX_EXTRA_DIR}"
        "DOXYGEN_XML_DIR=${DOXYGEN_XML_DIR}"
        ${SPHINX_EXECUTABLE} -j auto -E -a -T
        -Dbreathe_projects.Metkit=${DOXYGEN_XML_DIR}
        -Dversion=${METKIT_VERSION}
        -Drelease=${METKIT_VERSION}
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${SPHINX_OUTPUT_DIR}"
    DEPENDS ${DOXYGEN_INDEX_FILE} ${SPHINX_INPUT_FILES}
            ${PYMETKIT_SRCS}
            "${CMAKE_CURRENT_SOURCE_DIR}/conf.py"
    COMMENT "Running Sphinx to build HTML documentation"
)

# --- Target ---
add_custom_target(metkit-doc ALL DEPENDS ${SPHINX_INDEX_FILE} ${DOXYGEN_INDEX_FILE})
