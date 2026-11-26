Approach
========

This page gives the reasons behind the shape of the interface. How to use it
is on :doc:`usage`. The metkit API takes and returns eckit types: data
handles, messages, streams and configuration. Those, ``eckit::init`` and the
eckit error classes are documented in the Rust Interface chapter of the eckit
documentation.

Two Crates
----------

``metkit-sys``
   Unsafe bindings to the C++ library, built on `cxx <https://cxx.rs>`_.

   * :ref:`Builds libmetkit from source or links an installed one <rust/building:Building the Rust Crates>`.
   * :ref:`Exposes the metkit CMake options as Cargo features <rust/building:Cargo Features>`.
   * Converts metkit and eckit exceptions into typed errors.

``metkit``
   Safe Rust API over ``metkit-sys``.

   * :ref:`metkit::Result and a matchable metkit::Error <rust/usage:Errors>`.
   * :ref:`Expansion state as a type parameter <rust/approach:Request State>`.
   * Iterators for :ref:`parsed requests <rust/usage:Parsing MARS Text>`.
   * eckit types for :ref:`messages <rust/usage:Messages>`,
     :ref:`streams <rust/usage:Streams>` and
     :ref:`retrieval <rust/usage:MARS Retrieval>`.

Request State
-------------

The expansion state is a type parameter: ``MarsRequest<Raw>`` holds values
as given and is the only state with ``set``, ``unset`` and
``set_verb``; ``MarsRequest<Expanded>`` comes only from ``expand``, from the
parsers ``metkit::parse`` and ``str::parse`` or from ``from_trusted``, which
tags text as expanded on the caller's word. ``HyperCube::new`` and
``MarsRequestHandle::new`` take ``&MarsRequest<Expanded>``, so passing a raw
request does not compile.

Error Type
----------

``metkit::Error`` nests ``eckit::Error`` instead of copying its variants, so
an eckit exception thrown inside metkit is the same value it would be coming
from eckit itself. Rust code matches on the class the way C++ code catches
it.
