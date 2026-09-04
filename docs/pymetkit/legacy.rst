Legacy CFFI interface
=====================

.. deprecated:: 1.19.0
   The CFFI-based ``pymetkit`` package (under ``python/pymetkit``) is superseded by the
   pybind11-based package documented in :doc:`api`. It is retained for reference only and
   is no longer built, tested, or published.

.. warning::

   **Do not use the legacy CFFI interface for new code.** It wraps the ``metkit`` C API
   (``metkit_c.h``) through `cffi <https://cffi.readthedocs.io>`__ rather than binding the
   C++ classes directly, and it is not wired into the build or release workflows. All new
   development should target the Pythonic :doc:`api`.

Background
----------

The legacy interface exposed a ``MarsRequest`` class and a ``parse_mars_request`` function
backed by ``cffi`` (``ffi.dlopen`` + ``ffi.cdef`` against a stripped copy of
``metkit_c.h``), together with a ``PatchedLib`` error-wrapping layer and the
``MetKitException`` / ``CFFIModuleLoadFailed`` exceptions.

Installation
------------

.. warning::

   The legacy package is not published on PyPI and has no build system of its own.
   These steps are provided for reference only. All new code should use the
   pybind11-based ``pymetkit`` documented in :doc:`installation`.

The legacy CFFI interface requires ``metkit = 1.19.2``. The steps below build that
version from source and wire up the Python package.

**Build dependencies**

+----------+---------------------------------------------+
| Tool     | Link                                        |
+----------+---------------------------------------------+
| CMake    | https://cmake.org/                          |
+----------+---------------------------------------------+
| ecbuild  | https://github.com/ecmwf/ecbuild            |
+----------+---------------------------------------------+
| Ninja    | https://ninja-build.org/                    |
+----------+---------------------------------------------+

**Runtime dependencies**

+----------+---------------------------------------------+
| Library  | Link                                        |
+----------+---------------------------------------------+
| eckit    | https://github.com/ecmwf/eckit              |
+----------+---------------------------------------------+
| eccodes  | https://github.com/ecmwf/eccodes            |
+----------+---------------------------------------------+
| metkit   | https://github.com/ecmwf/metkit             |
+----------+---------------------------------------------+
| libaec   | https://github.com/MathisRosenhauer/libaec  |
+----------+---------------------------------------------+

**Python dependencies**

+--------------+---------------------------------------------+
| Requirement  | Link                                        |
+--------------+---------------------------------------------+
| Python 3.11  | https://www.python.org/                     |
+--------------+---------------------------------------------+
| cffi         | https://cffi.readthedocs.io                 |
+--------------+---------------------------------------------+
| findlibs     | https://github.com/ecmwf/findlibs           |
+--------------+---------------------------------------------+

**1. Build metkit 1.19.2 from source**

Create a bundle directory and switch to it:

.. code-block:: sh

   mkdir stack && cd stack

Place the following ``CMakeLists.txt`` in it:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

   find_package(ecbuild 3.8 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR} $ENV{HOME}/.local/ecbuild)

   project(ecmwf_stack_bundle VERSION 0.0.1 LANGUAGES CXX)

   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   ecbuild_bundle_initialize()

   ecbuild_bundle(PROJECT eckit   GIT "https://github.com/ecmwf/eckit"   BRANCH develop UPDATE)
   ecbuild_bundle(PROJECT eccodes GIT "https://github.com/ecmwf/eccodes" BRANCH develop UPDATE)
   ecbuild_bundle(PROJECT metkit  GIT "https://github.com/ecmwf/metkit"  TAG    1.19.2  UPDATE)

   ecbuild_bundle_finalize()

.. tip::

   If ``ecbuild``, ``ninja`` or ``aec`` are not available on your ``PATH``, load them via the
   environment modules system before running ``cmake``:

   .. code-block:: sh

      module load ecbuild ninja aec

   Alternatively, adjust the ``HINTS`` path in the ``find_package`` call to point at
   your ``ecbuild`` installation, and drop ``-G Ninja`` to fall back to ``make``.

Create a build directory, configure and compile:

.. code-block:: sh

   mkdir build && cd build
   cmake -DCMAKE_INSTALL_PREFIX=../install \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         .. -G Ninja
   ninja

**2. Set up a Python environment**

.. note::

   Python 3.11 is required. If it is not your system default, load it first:

   .. code-block:: sh

      module load python3/3.11

.. code-block:: sh

   cd ../..          # back to the stack root
   python3 -m venv .venv
   source .venv/bin/activate
   pip install cffi findlibs

``cffi`` provides the C-extension glue; ``findlibs``
(`<https://github.com/ecmwf/findlibs>`__) locates ``libmetkit`` at runtime.

**3. Install the legacy package**

The metkit 1.19.2 tag ships a ``pyproject.toml`` at its root that packages the
legacy CFFI interface. Install it directly from the checked-out source:

.. code-block:: sh

   cd stack/metkit
   pip install .
   cd ../..

Then let ``findlibs`` know where ``libmetkit`` was installed:

.. code-block:: sh

   export METKIT_DIR=stack/install

**4. Verify**

.. code-block:: sh

   python - <<'EOF'
   from pymetkit import MarsRequest
   req = MarsRequest("retrieve", class_="od", date="-1", param="130")
   print(list(req.keys()))
   EOF

Migration
---------

The new package keeps the same core concepts, so migration is largely mechanical:

- ``from pymetkit import MarsRequest, parse_mars_request`` — unchanged import surface.
- A request is now built from a verb and a plain mapping rather than keyword arguments:
  ``MarsRequest("retrieve", {"class": "od", "date": "-1"})`` replaces
  ``MarsRequest("retrieve", class_="od", date="-1")``.
- ``MarsRequest.expand``, ``validate``, ``merge``, ``keys``, ``__setitem__``,
  ``__contains__`` and ``__eq__`` behaviours are preserved.
- ``num_values`` and ``__getitem__`` now raise ``KeyError`` for a missing parameter
  instead of returning ``0`` / ``[]`` as the legacy C-API layer did.
- :data:`~pymetkit.pymetkit_type.MarsSelection` is now a type alias for a user-supplied
  mapping; value normalisation is handled internally rather than ad hoc.
- ``MetKitException`` is still raised for MARS language errors and is importable as
  ``pymetkit.MetKitException``.

Examples
^^^^^^^^

**Constructing a request**

The legacy interface accepted keyword arguments, using a trailing underscore to
escape Python reserved words such as ``class``:

.. code-block:: python

   # Legacy CFFI
   request = MarsRequest("retrieve", class_="od", date="-1", step=[0, 6, 12])

The new interface takes a plain mapping; no escaping is needed:

.. code-block:: python

   from pymetkit import MarsRequest
   request = MarsRequest("retrieve", {"class": "od", "date": "-1", "step": [0, 6, 12]})

**Missing parameters**

The legacy C-API returned sentinel values for absent parameters. The new interface
raises :exc:`KeyError` instead. Guard with ``in`` where absence is expected:

.. code-block:: python

   # Legacy: silent sentinel values
   n = request.num_values("step")  # returned 0 if not set
   v = request["step"]             # returned [] if not set

.. code-block:: python

   # New: raises KeyError — guard explicitly
   from pymetkit import MarsRequest
   request = MarsRequest("retrieve", {"class": "od", "date": "-1"})
   n = request.num_values("step") if "step" in request else 0
   v = request["step"]            if "step" in request else []

See :doc:`api` and :doc:`examples` for the current interface.
