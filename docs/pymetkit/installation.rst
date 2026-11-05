.. _installation-label:

Installation
############

Requirements
************

Build Dependencies
^^^^^^^^^^^^^^^^^^^

+----------+---------------------------------------------+
|Dependency|Link                                         |
+----------+---------------------------------------------+
|CMake     |http://www.cmake.org/                        |
+----------+---------------------------------------------+
|ecbuild   |https://github.com/ecmwf/ecbuild             |
+----------+---------------------------------------------+
|Pybind11  |https://pybind11.readthedocs.io              |
+----------+---------------------------------------------+

Runtime Dependencies
^^^^^^^^^^^^^^^^^^^^^

+----------+---------------------------------------------+
|Dependency|Link                                         |
+----------+---------------------------------------------+
|metkit    |https://github.com/ecmwf/metkit              |
+----------+---------------------------------------------+
|eckit     |https://github.com/ecmwf/eckit               |
+----------+---------------------------------------------+
|eccodes   |https://github.com/ecmwf/eccodes             |
+----------+---------------------------------------------+

``eccodes`` is optional. The CLI reports it as ``[Optional]`` and a missing
``eccodes`` does not cause a non-zero exit.

Python Dependencies
^^^^^^^^^^^^^^^^^^^

``pymetkit`` requires ``Python >= 3.11``. The runtime dependencies are pinned in
``python/requirements.txt``:

.. code-block:: sh

   uv pip install -r python/requirements.txt

``python/requirements-build.txt`` additionally covers everything needed to build the
documentation and run the doctests.

Build from sources (recommended)
********************************

``PyMetKit`` is built as part of ``metkit`` by enabling the
``ENABLE_PYTHON_METKIT_INTERFACE`` CMake option, which requires ``Python >= 3.11`` and
``pybind11 >= 3.0.1``.

Configure and build ``metkit`` (with its dependencies already installed at
``<prefix>``):

.. code-block:: sh

   cmake -B build -S . -G Ninja \
         -DCMAKE_PREFIX_PATH=<prefix> \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DENABLE_PYTHON_METKIT_INTERFACE=ON
   cmake --build build -j

This produces the ``pymetkit`` wheel in the build directory and a ready-to-use package
layout under ``build/pymetkit-python-package-staging``.

.. tip::

   For local exploration you can put the staging directory on your ``PYTHONPATH``
   instead of installing the wheel:

   .. code-block:: sh

      export PYTHONPATH=<path-to-build>/pymetkit-python-package-staging

Run the tests to verify the build:

.. code-block:: sh

   cd build
   ctest --output-on-failure -L pymetkit

Installation via PyPI
*********************

.. code-block:: sh

   uv venv
   source .venv/bin/activate
   uv pip install pymetkit

Set the ``METKIT_HOME`` environment variable so ``findlibs`` can locate the library:

.. code-block:: sh

   export METKIT_HOME=<path_to_metkit_home>

Diagnosing Library Resolution
*****************************

``PyMetKit`` uses `findlibs <https://github.com/ecmwf/findlibs>`__ to locate the
``metkit`` shared library and its runtime dependencies at import time. If you encounter
errors caused by the wrong library version being loaded, the built-in CLI can help you
inspect what ``findlibs`` resolves on your system.

Print the installation root of the ``metkit`` library:

.. code-block:: sh

   python -m pymetkit --print-home

Print the resolved home directories for all runtime dependencies (``eckit``,
``eccodes``, ``metkit``), together with any active ``FINDLIBS_DISABLE_*`` environment
variables that suppress specific search paths:

.. code-block:: sh

   python -m pymetkit --print-home-deps

Add ``-v`` / ``--verbose`` to either command to raise the log level from ``INFO`` to
``DEBUG``.

``ERROR`` lines indicate dependencies ``findlibs`` could not locate — set the
corresponding ``<LIBNAME>_HOME`` environment variable to resolve them explicitly.

The exit code tells you whether the resolution succeeded:

+---------+--------------------------------------------------------------+
|Exit code|Meaning                                                       |
+---------+--------------------------------------------------------------+
|0        |All required dependencies resolved. A missing optional        |
|         |dependency (``eccodes``) still exits ``0``.                   |
+---------+--------------------------------------------------------------+
|1        |A required dependency could not be located.                   |
+---------+--------------------------------------------------------------+
|2        |No flag given; the help text is printed instead.              |
+---------+--------------------------------------------------------------+
