# (C) Copyright 2025- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

import datetime

project = "PyMetKit"
copyright = f"{datetime.datetime.today().year}, ECMWF"
author = "ECMWF"

extensions = [
    "sphinx.ext.autosectionlabel",
    "sphinxcontrib.mermaid",
    "autoapi.extension",
    "sphinx.ext.viewcode",
    "sphinx.ext.napoleon",
    "sphinx.ext.autodoc",
    "sphinx.ext.doctest",
    "sphinx.ext.inheritance_diagram",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store", "_internal"]

# -- sphinx-autoapi: statically parse the pythonic pymetkit package.
# The compiled `pymetkit_bindings` layer has no Python source and is documented
# by hand in `bindings.rst`. The `_internal` glue package is hidden.
autoapi_dirs = ["../../src/pymetkit"]
autoapi_type = "python"
autoapi_generate_api_docs = True
autoapi_add_toctree_entry = False
autoapi_python_class_content = "class"
autoapi_ignore = [
    "*/_internal/*",
    "*/__main__.py",
]
add_module_names = False
autoapi_keep_files = False

# -- Napoleon settings (pymetkit docstrings are NumPy-style)
napoleon_google_docstring = False
napoleon_numpy_docstring = True

html_theme = "pydata_sphinx_theme"
html_show_sourcelink = False
html_sidebars = {"**": []}
html_theme_options = {
    "navbar_align": "left",
    "navbar_start": ["navbar-logo"],
    "navbar_center": ["navbar-nav"],
    "navbar_end": ["navbar-icon-links", "theme-switcher", "version-switcher"],
    "navbar_persistent": ["search-button"],
    "primary_sidebar_end": [],
    "check_switcher": False,
}
html_context = {"default_mode": "auto"}
autosectionlabel_prefix_document = True
