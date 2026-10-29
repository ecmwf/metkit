Approach
========

Every call ends up in the same metkit code that C++ applications run, so the
MARS language, the expansion rules and the configuration files are the same in
both languages.

The interface is built the same way as the eckit Rust interface, and reuses
its pieces rather than repeating them: the error type, the logging bridge and
the data handle, message, configuration and stream types all come from the
``eckit`` and ``eckit-sys`` crates. That shared design is described in the
Rust Interface section of the eckit documentation, kept under
``docs/content/rust`` in the `eckit repository
<https://github.com/ecmwf/eckit>`__. This page covers what is specific to
metkit.

Two crates
----------

.. mermaid::

   flowchart TD
       app["Rust application"]
       metkit["metkit<br/>safe API"]
       sys["metkit-sys<br/>cxx bridge, generated errors, build script"]
       shim["C++ shims<br/>namespace metkit_bridge"]
       cpp["metkit C++ library"]
       eckit["eckit-sys"]
       eccodes["eccodes-sys"]
       other["other -sys crates<br/>for packages built on metkit, e.g. fdb-sys"]

       app --> metkit
       metkit --> sys
       other --> sys
       sys --> shim
       shim --> cpp
       sys --> eckit
       sys --> eccodes

``metkit-sys`` (low level)
   Owns everything that touches C++:

   * a build script that compiles the metkit sources with ecbuild, or finds an
     installed metkit, and emits the link directives. It builds against the
     eckit and ecCodes that ``eckit-sys`` and ``eccodes-sys`` built or found,
     which it reads from the ``DEP_ECKIT_SYS_ROOT`` and
     ``DEP_ECCODES_SYS_ROOT`` variables those crates export. One copy of each
     library ends up in the program;
   * the ``cxx`` bridge in ``src/lib.rs``, declaring the functions Rust may
     call;
   * thin C++ wrapper classes in ``rust/crates/metkit-sys/cpp``, one per
     metkit class, that adapt metkit signatures to the types ``cxx`` can
     pass;
   * the ``Error`` enum generated from metkit's exception header;
   * a `bindman <https://github.com/ecmwf/bindman>`__ annotation on the bridge
     naming the C++ headers it wraps, ``MarsRequest.h`` and
     ``MarsLanguage.h``. The build script compares those headers against the
     bridge, so a method added to or removed from the C++ class is reported
     when the crate is built.

``metkit`` (high level)
   Contains no C++. It wraps the ``metkit-sys`` types in an API that follows
   Rust conventions: fallible calls return ``metkit::Result<T>``,
   ``ParsedRequests`` is an iterator, and the expansion state of a request is
   a type parameter.

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Wrapper
     - Wraps
   * - ``MarsRequestWrapper``
     - ``metkit::mars::MarsRequest``
   * - ``ParsedRequestsWrapper``
     - The ``std::vector<MarsRequest>`` returned by ``MarsRequest::parse`` or
       ``MarsParser::parse``
   * - ``CodesHandleWrapper``
     - ``metkit::codes::CodesHandle``
   * - ``HyperCubeWrapper``
     - ``metkit::hypercube::HyperCube``
   * - ``RequestEnvironmentWrapper``
     - ``metkit::mars::RequestEnvironment``
   * - ``MarsLanguageWrapper``
     - ``metkit::mars::MarsLanguage``

Rust owns a wrapper through ``cxx::UniquePtr``, so the C++ object is destroyed
when the Rust value is dropped.

Request state in the type system
--------------------------------

A C++ ``MarsRequest`` does not record whether the MARS language has expanded
it. Code that needs canonical values, such as the ``HyperCube`` constructor or
``MarsRequestHandle``, assumes expansion has run; a raw request passed there is
a run-time failure, or a request the server reads differently from what the
caller meant.

In Rust the state is a type parameter. ``MarsRequest<Raw>`` holds values as
given and is the only state with ``set``, ``unset`` and ``set_verb``.
``MarsRequest<Expanded>`` is produced by metkit's own expansion and nothing
else:

* ``MarsRequest::expand``,
* ``metkit::parse`` and ``str::parse``, which run the expansion as part of
  parsing,
* ``MarsRequest::from_trusted``, where the caller asserts the text is already
  canonical.

``HyperCube::new`` and ``MarsRequestHandle::new`` take
``&MarsRequest<Expanded>``, so passing a raw request does not compile. Editing
an expanded request goes through ``into_raw``, which re-tags the value without
an FFI call, and back through ``expand``. ``set_trusted`` edits in place and
keeps the tag, on the caller's authority. Operations that read a request, such
as ``get``, ``values``, ``matches`` and ``encode``, are available in both
states. The approach is the one ``eckit::DataHandle`` takes with its
``Closed``, ``Reading`` and ``Writing`` states.

Errors
------

Metkit throws its own exception classes and eckit's. Every exception is caught
at the bridge and turned into a value of one ``Error`` enum, keeping the
class so that Rust code can match on it.

The ``metkit-sys`` build script parses ``metkit/codes/api/CodesTypes.h`` and
generates two variants, ``CodesException`` and ``CodesWrongLength``, with a
``Display`` that keeps the class name: ``metkit::CodesException: ...``. It also
reads the list of exception headers that ``eckit-sys`` publishes and adds the
eckit ``catch`` blocks to its own handler, so an eckit exception thrown inside
metkit reaches Rust as the same ``eckit::Error`` variant it would have had
coming from eckit itself. The generated handler for this release covers 32
classes: metkit's two and eckit's 30.

The high-level crate combines the two:

.. code-block:: rust

   pub enum Error {
       Metkit(metkit_sys::Error),   // CodesException, CodesWrongLength
       Eckit(eckit::Error),         // UserError, CantOpenFile, ...
       Other(String),               // anything else, and errors raised in Rust
   }

Expansion failures are ``eckit::UserError``. ``MarsLanguage`` throws it for a
keyword or value that matches nothing in the language
(``Cannot match [foo] in [...]``), a verb it does not support, and for any
other failure during expansion, with the request appended to the message.
Reading a GRIB key that does not exist is a ``CodesException``.

Matching on the inner variants needs the ``metkit-sys`` and ``eckit`` crates
as direct dependencies of the application, since the enum types are theirs.
``Error`` implements ``std::error::Error`` and ``Display``, so it works with
``?``, ``Box<dyn Error>`` and error-reporting crates without that.

Shared eckit types
------------------

Metkit's C++ API takes and returns eckit objects, and the Rust API keeps them
as the eckit crate's types:

* ``MarsRequest::encode`` and ``MarsRequest::decode`` take
  ``&mut dyn eckit::Stream``, and ``MarsRequest`` implements
  ``eckit::StreamWrite``. The bytes are those of the C++ ``operator<<`` on
  ``eckit::Stream``, so a request written in Rust is read by C++ and the other
  way round.
* ``MarsRequest`` implements ``TryFrom<&eckit::Message>``, the C++
  constructor that reads a request from the metadata of a GRIB message.
* ``MarsRequestHandle`` is an ``eckit::DataHandle``: opening it gives a
  ``DataHandle<Reading>`` with ``std::io::Read``, which ``eckit::MessageReader``
  iterates.
* ``MarsRequestHandle::new`` takes an ``eckit::Config``.

Linking metkit also changes what eckit can do. The GRIB and BUFR splitters and
decoders that ``eckit::message`` dispatches to are registered by metkit when
the library loads, so an ``eckit::MessageReader`` finds GRIB messages only in
a program that links metkit with the ``grib`` feature.

In the other direction, ``MarsRequest::as_sys`` and ``CodesHandle::as_sys``
lend the wrapper to a bridge that expects a metkit object. The ``fdb`` crate
takes ``&metkit::MarsRequest<S>`` for its list, retrieve and administrative
calls and passes it across the FDB bridge this way.

On the build side, ``metkit-sys`` declares ``links = "metkit_sys"`` and exports
to the build scripts of crates that depend on it:

.. list-table::
   :widths: 40 60
   :header-rows: 1

   * - Variable
     - Content
   * - ``DEP_METKIT_SYS_ROOT``
     - Install prefix of the metkit that was built or found
   * - ``DEP_METKIT_SYS_INCLUDE``
     - The metkit include directory
   * - ``DEP_METKIT_SYS_CPP_DIR``
     - Directory holding ``MetkitBridge.h`` and the wrapper headers

Logging
-------

Metkit writes diagnostics through ``eckit::Log``. ``eckit::init()`` routes
those channels to the Rust ``log`` crate, so metkit's output goes to whatever
backend the application installs. The library's debug channel uses the target
``metkit``, which an ``env_logger`` filter such as ``RUST_LOG=metkit=debug``
selects on its own.

Extending the bindings
----------------------

To expose another part of metkit:

#. Add a wrapper class to ``rust/crates/metkit-sys/cpp`` in the
   ``metkit_bridge`` namespace. Include ``metkit_exceptions.h`` first in the
   ``.cc`` file so exceptions are translated.
#. Declare the wrapper and its methods in the ``cxx`` bridge in
   ``rust/crates/metkit-sys/src/lib.rs``. Methods that can throw return
   ``Result``. If the wrapper covers a metkit class the bridge did not track
   before, add its header to the ``track_cpp_api`` list.
#. List the new sources in ``rust/crates/metkit-sys/build.rs``.
#. Add the safe API to ``rust/crates/metkit``, converting errors with
   ``metkit::Error::from`` and providing ``as_sys`` if other packages need to
   pass the type across their own bridges.
