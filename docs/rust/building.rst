Building the Rust crates
========================

The crates build with Cargo. CMake and ecbuild are driven by the build script
of ``metkit-sys`` and do not need to be run by hand.

Toolchain requirements
----------------------

* Rust 1.90 or later
* A C++17 compiler
* CMake and git

Vendored and system builds
--------------------------

Two Cargo features select where the C++ libraries come from. They are mutually
exclusive, and the one chosen for metkit is forwarded to ``eckit-sys`` and
``eccodes-sys``, so all three libraries come from the same place.

``vendored`` (default)
   Builds metkit from source into Cargo's output directory, using ecbuild,
   which is downloaded automatically. When the crate is used from a checkout
   of the metkit repository, by path or as a git dependency, the C++ sources
   of that same checkout are compiled, so Rust and C++ changes can be
   developed and tested together; the build prints a warning when the
   checkout's ``VERSION`` differs from the crate version. Otherwise the
   release tag matching the ``metkit-sys`` version is cloned.

``system``
   Links an installed metkit, located with CMake ``find_package(metkit)``.
   Requires metkit 1.18.1 or later. Set ``METKIT_DIR`` or
   ``CMAKE_PREFIX_PATH`` if it is not in a default location, and likewise
   ``ECKIT_DIR`` and ``ECCODES_DIR`` for its dependencies:

   .. code-block:: toml

      [dependencies]
      metkit = { version = "1.22", default-features = false, features = ["system", "grib", "bufr"] }

   .. code-block:: bash

      METKIT_DIR=/path/to/metkit/install cargo build

Use ``system`` when the program must share one installation with other
software, for example on the HPC or in a bundle build. Use ``vendored`` for a
self-contained build.

The shared libraries of a vendored build are found at run time through rpath
entries on the final binary. An application adds them from its build script
with ``bindman_utils::emit_rpaths()``:

.. code-block:: toml

   [build-dependencies]
   bindman-utils = "0.2"

.. code-block:: rust

   // build.rs
   fn main() {
       bindman_utils::emit_rpaths();
   }

Run-time files
--------------

The MARS language lives in YAML files, ``language.yaml`` among them, that
metkit reads when it first expands a request. The library looks for them in
``~metkit/share/metkit``, a path eckit resolves to the install prefix of the
metkit library it loaded, or to the ``METKIT_HOME`` environment variable when
that is set. A vendored build installs the files next to the library it
built, under Cargo's output directory, and a system build uses those of the
installation. The ``metkit-config`` feature of ``metkit-sys``, on by default,
is what installs them in a vendored build; without it, or without the files,
expansion fails.

Crate versions
--------------

Both crates carry the version of the metkit release they wrap. The version is
set once for the workspace in ``rust/Cargo.toml``, and a test in each crate
fails if it drifts from the ``VERSION`` file of the repository.

Cargo features
--------------

The ``metkit`` crate has four features:

.. list-table::
   :widths: 25 15 60
   :header-rows: 1

   * - Feature
     - Default
     - Effect
   * - ``vendored``
     - on
     - Build metkit, eckit and ecCodes from source
   * - ``system``
     - off
     - Link installed libraries
   * - ``grib``
     - on
     - GRIB support, through ecCodes: ``CodesHandle`` and the GRIB splitter
       and decoder
   * - ``bufr``
     - on
     - BUFR support, through ecCodes

``metkit-sys`` exposes the CMake options of metkit as features, so a vendored
build can be configured from ``Cargo.toml``. A feature switches on the
``ENABLE_*`` option of the same name.

.. list-table::
   :widths: 25 15 60
   :header-rows: 1

   * - Feature
     - Default
     - Effect
   * - ``grib``, ``bufr``
     - on
     - Format support; each enables the matching product in ``eccodes-sys``
   * - ``mars2grib``
     - on
     - The MARS2GRIB encoder; enables ``geo-codec-grids`` in ``eckit-sys``
       for ORCA, FESOM and ICON grids
   * - ``metkit-config``
     - on
     - Install the MARS language and parameter files
   * - ``netcdf``
     - off
     - NetCDF data support; needs a system NetCDF
   * - ``odb``
     - off
     - ODB data support; needs odc
   * - ``experimental``, ``fail-on-ccsds``
     - off
     - Experimental upstream features; refuse CCSDS-packed messages

To switch one on, add ``metkit-sys`` as a dependency next to ``metkit`` and
enable the feature there:

.. code-block:: toml

   [dependencies]
   metkit = "1.22"
   metkit-sys = { version = "1.22", features = ["netcdf"] }

Build environment variables
---------------------------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Variable
     - Effect
   * - ``METKIT_DIR``
     - Install prefix of metkit, used by ``system`` builds
   * - ``ECKIT_DIR``, ``ECCODES_DIR``
     - Install prefixes of the dependencies, used by their ``system`` builds
   * - ``CMAKE_PREFIX_PATH``
     - Additional CMake search paths
   * - ``DOCS_RS``
     - When set, the C++ build is skipped and only the generated error type is
       produced, which is enough for ``cargo doc``

Working on the crates
---------------------

All commands run from the ``rust/`` directory of the repository:

.. code-block:: bash

   cargo clippy --all-targets     # lint; the workspace enables pedantic lints
   cargo fmt                      # format Rust sources
   cargo test                     # tests
   cargo doc --no-deps --open     # API reference

The first build compiles the three C++ libraries. Later builds reuse them and
recompile only when C++ sources change.

The programs in ``docs/rust/examples`` are the code shown in these pages.
They are complete ``main`` programs and build as examples of a package that
depends on the ``metkit`` and ``eckit`` crates.
