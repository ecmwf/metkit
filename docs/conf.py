# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html
import datetime
import os

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information


project = "Metkit"
copyright = f"{datetime.datetime.today().year}, ECMWF"
author = "ECMWF"
version = "local-dev"
release = "local-dev"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.autosectionlabel",
    "sphinxcontrib.mermaid",
    "autoapi.extension",
    "sphinx.ext.viewcode",
    "sphinx.ext.napoleon",
    "sphinx.ext.autodoc",
    "sphinx.ext.doctest",
    "sphinx.ext.inheritance_diagram",
    "breathe",
    "myst_parser",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", "_internal"]

autoapi_dirs = ["../python/pymetkit/src/pymetkit"]
autoapi_type = "python"
autoapi_generate_api_docs = True
autoapi_add_toctree_entry = False
autoapi_python_class_content = "both"
autoapi_ignore = [
    "*/_internal/*",
]
add_module_names = False
autoapi_keep_files = False

# -- Napoleon settings
napoleon_google = True

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "pydata_sphinx_theme"
html_context = {"default_mode": "auto"}
html_theme_options = {
    "switcher": {
        "json_url": "https://sites.ecmwf.int/docs/metkit/versions.json",
        "version_match": version,
    },
    "show_toc_level": 2,
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/ecmwf/metkit",
            "icon": "fa-brands fa-github",
        },
    ],
    "navbar_align": "left",
    "navbar_start": ["navbar-logo"],
    "navbar_center": ["navbar-nav"],
    "navbar_end": ["navbar-icon-links", "theme-switcher", "version-switcher"],
    "navbar_persistent": ["search-button"],
    "secondary_sidebar_items": ["page-toc", "edit-this-page", "sourcelink"],
    # On local builds no version.json is present
    "check_switcher": False,
    "content_footer_items": [],
    "footer_start": ["copyright"],
    "footer_center": ["sphinx-version"],
    "footer_end": ["theme-version"],
}
html_sidebars = {"**": ["sidebar-nav-bs"]}
html_static_path = ["_static"]

# -- Breathe configuration ---------------------------------------------------
breathe_projects = {"Metkit": os.environ.get("DOXYGEN_XML_DIR", "doxygen/xml")}
breathe_default_project = "Metkit"

# -- Doxygen HTML ------------------------------------------------------------
# The full Doxygen-generated HTML (indexes, tables, class/file lists) is copied
# verbatim into the output under ``doxygen/`` so it can be surfaced under the
# "Doxygen" section. ``DOXYGEN_HTML_EXTRA_DIR`` must point at a directory that
# contains a ``doxygen/`` subfolder with the generated HTML (see build_docs.sh
# and docs/CMakeLists.txt).
_doxygen_html_extra_dir = os.environ.get("DOXYGEN_HTML_EXTRA_DIR")
html_extra_path = []
if _doxygen_html_extra_dir and os.path.isdir(_doxygen_html_extra_dir):
    html_extra_path.append(_doxygen_html_extra_dir)

# -- autosectionlabel configuration ------------------------------------------
autosectionlabel_prefix_document = True
