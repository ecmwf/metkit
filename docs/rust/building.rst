Building the Rust Crates
========================

.. code-block:: bash

   cargo add metkit eckit
   cargo build

This builds metkit, eckit and ecCodes from source as part of ``cargo build``
(the ``vendored`` feature, on by default). It needs Rust 1.90 or later, a
C++17 compiler, CMake and git. The first build also needs network access, to
download ecbuild and the releases matching the crate versions. When the crate
comes from a checkout of the metkit repository, as a path or git dependency,
the C++ sources of that checkout are compiled instead.

To link an installed metkit, 1.18.1 or later, switch both crates to the
``system`` feature and point CMake at the installations with
``METKIT_DIR``, ``ECKIT_DIR`` and ``ECCODES_DIR`` or ``CMAKE_PREFIX_PATH``:

.. code-block:: bash

   cargo add metkit --no-default-features --features system,grib,bufr
   cargo add eckit --no-default-features --features system
   METKIT_DIR=/path/to/metkit/install cargo build

Use ``system`` when the program must share one installation with other
software, for example on the HPC or in a bundle build and ``vendored`` for a
self-contained build.

Running the Examples
--------------------

The programs quoted in :doc:`usage` are examples of the ``metkit`` crate, in
``rust/crates/metkit/examples``. From a checkout of the repository:

.. code-block:: bash

   cd rust
   cargo run -p metkit --example quickstart
   cargo run -p metkit --example requests

Language Files
--------------

Expansion reads the MARS language from YAML files under ``share/metkit``. A
vendored build installs them with the library. A system build finds them in
the metkit installation or under ``METKIT_HOME`` when that is set. Without
the files, expansion fails.

Cargo Features
--------------

Besides ``vendored`` and ``system``, the ``metkit`` crate has ``grib`` and
``bufr``, both on by default. ``grib`` adds ``CodesHandle`` and GRIB
decoding; ``bufr`` adds BUFR decoding.

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
     - GRIB and BUFR support
   * - ``mars2grib``
     - off
     - The MARS2GRIB encoder
   * - ``metkit-config``
     - on
     - Install the MARS language and parameter files
   * - ``netcdf``
     - off
     - NetCDF data support; needs a system NetCDF
   * - ``odb``
     - off
     - ODB data support; needs odc
   * - ``experimental``
     - off
     - Experimental upstream features
   * - ``fail-on-ccsds``
     - off
     - Refuse CCSDS-packed messages

To switch one on, add ``metkit-sys`` as a dependency next to ``metkit`` and
enable the feature there:

.. code-block:: bash

   cargo add metkit
   cargo add metkit-sys --features netcdf
