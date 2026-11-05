.. _Rust_Introduction:

Rust Interface
==============

:Version: |version|

The Rust interface lives in the ``rust/`` directory of the metkit repository
and calls the metkit C++ library. This chapter is for developers writing
applications against the crates.

``metkit`` is the crate applications depend on; it gives the C++ library a
Rust API, with a ``MarsRequest`` type that records whether the MARS language
has expanded it and a ``metkit::Error`` enum to match on. ``metkit-sys``
builds or links the C++ library underneath it.

.. note::

   Use ``metkit`` in your code. ``metkit-sys`` goes into a ``Cargo.toml``
   only to switch on a build feature of the C++ library (:doc:`building`);
   calling it directly means handling ``cxx`` pointers and raw exception
   strings yourself.

Coverage
--------

.. list-table::
   :widths: 32 33 35
   :header-rows: 1

   * - Rust
     - C++
     - Notes
   * - ``metkit::MarsRequest<Raw>``,
       ``metkit::MarsRequest<Expanded>``
     - ``metkit::mars::MarsRequest``
     - Build, read, expand, encode and decode a request
   * - ``metkit::parse``, ``metkit::tokenize``, ``metkit::ParsedRequests``
     - ``MarsRequest::parse``, ``MarsParser``
     - Parse MARS text holding one or more requests, with or without
       expansion
   * - ``metkit::HyperCube``
     - ``metkit::hypercube::HyperCube``
     - Count the fields a request describes and tick them off as they arrive
   * - ``metkit::CodesHandle``
     - ``metkit::codes::CodesHandle``
     - Typed access to the keys of one GRIB message; feature ``grib``, on by
       default
   * - ``metkit::MarsRequestHandle``
     - ``metkit::mars::MarsRequestHandle``
     - An ``eckit::DataHandle`` that retrieves a request over the MARS client
       protocol
   * - ``metkit::initialize``,
       ``metkit::environment::environment_request``
     - ``metkit::mars::RequestEnvironment``
     - The client metadata sent with every protocol request
   * - ``metkit::Error``
     - ``metkit::codes::CodesException``, ``CodesWrongLength`` and the
       ``eckit::Exception`` classes
     - One variant per metkit exception class, ``Eckit`` for eckit's and
       ``Other``

Quick Start
-----------

Add the crates:

.. code-block:: bash

   cargo add metkit eckit

The crate version is the version of the metkit release it wraps. ``eckit``
provides ``eckit::init`` and the data handles, messages and streams that the
metkit API takes and returns. :doc:`building` lists the requirements and the
alternative of linking installed libraries.

.. literalinclude:: ../../rust/crates/metkit/examples/quickstart.rs
   :language: rust
   :start-after: // [main]
   :end-before: // [end main]

.. toctree::
   :maxdepth: 2
   :caption: Contents

   approach
   usage
   building

The per-item API reference is on docs.rs: `metkit <https://docs.rs/metkit>`_
and `metkit-sys <https://docs.rs/metkit-sys>`_.
