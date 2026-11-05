Development
###########

Follow the guide in :ref:`installation-label`. We advise using ``uv`` for installing the
build dependencies.

The version pinning of the ``metkitlib`` dependency is disabled by default, which is what
you want for local development. On CI it is enabled by setting the ``VERSION_SUFFIX``
environment variable, which pins the matching ``metkitlib`` wheel version.

To use your local ``metkit`` build, make sure ``findlibs`` is installed in your ``venv``
and export:

.. code-block:: sh

   export FINDLIBS_DISABLE_PACKAGE=yes

Set the ``METKIT_HOME`` environment variable to the build folder so ``findlibs`` picks up
the correct ``metkit`` library:

.. code-block:: sh

   export METKIT_HOME=<path_to_build_folder>

You can then install the ``pymetkit`` wheel from the build folder (or the staging
directory) into your ``venv``:

.. code-block:: sh

   uv pip install pymetkit-<version>-cp311-cp311-<platform>.whl
   # or, for an editable-style install of the staged package
   uv pip install -e pymetkit-python-package-staging

Run the tests by switching to the ``pymetkit`` tests folder and executing ``pytest``:

.. code-block:: sh

   cd <path_to_metkit>/tests/pymetkit
   pytest

The code examples in :doc:`examples` are executed as tests via
`sybil <https://sybil.readthedocs.io>`__. They run from the build directory under their
own ctest label, so you can exercise them separately from the unit tests:

.. code-block:: sh

   cd build
   ctest --output-on-failure -L pymetkit_doc

Building the documentation
**************************

Install the build requirements and run the build script:

.. code-block:: sh

   uv pip install -r python/requirements-build.txt
   ./docs/pymetkit/build_docs.sh

The rendered documentation is written to ``docs/pymetkit/doc-build/sphinx``. Pass a path
as the first argument to write it elsewhere.
