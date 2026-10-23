Using the ``metkit`` crate
==========================

Every fallible call in the high-level crate returns ``metkit::Result<T>``, an
alias for ``Result<T, metkit::Error>``; see :doc:`approach` for how the error
type relates to metkit and eckit exceptions. The programs on this page are in
``docs/rust/examples`` and build as described in :doc:`building`.

Initialisation
--------------

Call ``eckit::init()`` once, before anything else. It sets up the eckit
runtime that metkit's resources and logging go through, and routes
``eckit::Log`` output to the Rust ``log`` crate. Repeated calls are harmless.

A program that retrieves data through the MARS client protocol also sets the
client metadata that is sent with every request:

.. code-block:: rust

   eckit::init();
   metkit::initialize(&[("client", "my-tool"), ("version", "0.1")]);

``metkit::environment_request()`` returns that metadata as a
``MarsRequest``.

Building requests
-----------------

``MarsRequest`` wraps ``metkit::mars::MarsRequest``. A new request is
``MarsRequest<Raw>``, which is mutable:

.. literalinclude:: examples/requests.rs
   :language: rust
   :start-after: // [build]
   :end-before: // [end build]
   :dedent:

``set`` takes the value in the forms the C++ ``setValue`` overloads accept,
and the last write to a key wins:

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - Value type
     - Stored as
   * - ``&str``, ``String``, ``&String``
     - One value
   * - ``i64``
     - One value, formatted as decimal
   * - ``Vec<String>``, ``Vec<&str>``, ``&[String]``, ``&[&str]``,
       ``[&str; N]``
     - One value per element

Reading works in either state. ``get`` returns the value of a key that has
exactly one, and fails with ``eckit::UserError`` when the key has several;
``values`` returns all of them. ``params`` lists the keywords, and ``count``
is the number of fields the request describes: the product of the value
counts, with the C++ ``count`` rules for model levels and for ``all`` and
``any``. ``Display`` prints the same text as the C++ ``dump``, and
``to_json`` the parameters as a JSON object without the verb:

.. literalinclude:: examples/requests.rs
   :language: rust
   :start-after: // [read]
   :end-before: // [end read]
   :dedent:

Expansion
---------

``expand`` runs the MARS language over the request and returns a new
``MarsRequest<Expanded>``: aliases are replaced by canonical values (``t``
becomes ``130``), relative dates are resolved, ranges such as ``0/to/12/by/6``
are enumerated, and keywords are reordered into the language's axis order.

.. literalinclude:: examples/requests.rs
   :language: rust
   :start-after: // [expand]
   :end-before: // [end expand]
   :dedent:

The two flags are those of ``MarsLanguage::expand``:

``inherit``
   When set, keywords the request does not mention are filled with the
   defaults of the language. A ``retrieve`` with only ``class=od`` and a date
   gains ``type=an``, ``stream=oper``, ``levtype=pl``, ``time=1200``,
   ``step=0``, ``expver=0001``, ``domain=g``, six pressure levels and
   ``param=129``. When unset, the request keeps only the keywords it had.

``strict``
   When set, a keyword that the language rules out in the context of the
   other keywords, or a keyword those rules require but the request lacks,
   is an error. When unset, the keyword is removed or filled in silently.

Matching a value against the language is exact by default. Setting the
``METKIT_LANGUAGE_STRICT_MODE`` resource to false allows a value to match an
entry it abbreviates.

Parsing MARS text
-----------------

``metkit::parse`` reads text holding one or more requests, expands each, and
returns ``ParsedRequests<Expanded>``, which is iterated or collected:

.. literalinclude:: examples/requests.rs
   :language: rust
   :start-after: // [parse]
   :end-before: // [end parse]
   :dedent:

``str::parse`` does the same for text that holds exactly one request, and
is what the quick start on :doc:`index` uses. The ``strict`` argument of
``parse`` is the ``strict`` flag of expansion.

``metkit::tokenize`` runs the parser alone, with no expansion and no check
of the verb, so it accepts ``environ`` and other verbs the language does not
define. Its results are ``MarsRequest<Raw>``:

.. literalinclude:: examples/requests.rs
   :language: rust
   :start-after: // [tokenize]
   :end-before: // [end tokenize]
   :dedent:

Trusted input
-------------

Text that is known to be canonical already, for example a request a server
produced, can skip expansion with ``from_trusted``. It is parsed and tagged
``Expanded`` on the caller's word; a value that is not canonical surfaces
later, in whatever C++ code consumes the request. ``set_trusted`` edits an
expanded request in place under the same contract:

.. literalinclude:: examples/requests.rs
   :language: rust
   :start-after: // [trusted]
   :end-before: // [end trusted]
   :dedent:

The checked way to edit an expanded request is to re-tag it as raw, which
costs nothing, edit it, and expand it again:

.. literalinclude:: examples/requests.rs
   :language: rust
   :start-after: // [edit]
   :end-before: // [end edit]
   :dedent:

Counting fields with ``HyperCube``
----------------------------------

``HyperCube`` wraps ``metkit::hypercube::HyperCube``. Built from an expanded
request, it enumerates the fields the request describes, one cell per
combination of values, and keeps track of which cells have been seen.
``clear`` marks a cell and returns whether it was still vacant, and
``field_ordinal`` gives the position of a field in the request's order:

.. literalinclude:: examples/hypercube.rs
   :language: rust
   :start-after: // [cube]
   :end-before: // [end cube]
   :dedent:

``HyperCube::new`` takes ``&MarsRequest<Expanded>``. ``contains``, ``clear``
and ``field_ordinal`` take a request in either state, since a request built
from a GRIB message is ``Raw``.

GRIB keys with ``CodesHandle``
------------------------------

``CodesHandle`` wraps ``metkit::codes::CodesHandle``, one GRIB message held by
ecCodes. It is available with the ``grib`` Cargo feature, on by default. A
handle comes from a file (``from_file``, ``from_file_at_offset``), from bytes
(``from_message``, which copies them) or from an ecCodes sample
(``from_sample``). Keys are read and written by name, and the type asked for
selects the ecCodes accessor:

.. list-table::
   :widths: 30 35 35
   :header-rows: 1

   * - Rust type
     - ``get``
     - ``set``
   * - ``String``, ``&str``
     - ``codes_get_string``
     - ``codes_set_string``
   * - ``i64``
     - ``codes_get_long``
     - ``codes_set_long``
   * - ``f64``
     - ``codes_get_double``
     - ``codes_set_double``
   * - ``Vec<f64>``, ``&[f64]``
     - ``codes_get_double_array``
     - ``codes_set_double_array``
   * - ``Vec<i64>``
     - ``codes_get_long_array``
     - not available

.. literalinclude:: examples/codes.rs
   :language: rust
   :start-after: // [codes]
   :end-before: // [end codes]
   :dedent:

``message_data`` borrows the encoded message from the handle; copy it, or
build a second handle with ``try_clone``, to keep it past the handle's life.
``is_defined``, ``is_missing`` and ``has`` distinguish a key that is absent,
present but coded as missing, and present with a value; ``set_missing``
codes a key as missing.

Messages and streams
--------------------

A request can be built from the metadata of a GRIB message that
``eckit::MessageReader`` found in a data handle. The result is
``MarsRequest<Raw>``, since GRIB keys are not language-expanded:

.. literalinclude:: examples/codes.rs
   :language: rust
   :start-after: // [messages]
   :end-before: // [end messages]
   :dedent:

The reader finds GRIB messages because metkit registers the GRIB splitter
and decoder with ``eckit::message`` when the library loads.

``encode`` and ``decode`` move a request through an ``eckit::Stream`` in the
wire format of the C++ ``operator<<``, so a request written in Rust is read by
C++ and the other way round. ``MarsRequest`` also implements
``eckit::StreamWrite``, for code that serialises through that trait. A
decoded request is ``Raw``, since the wire carries no record of expansion:

.. literalinclude:: examples/stream.rs
   :language: rust
   :start-after: // [stream]
   :end-before: // [end stream]
   :dedent:

Retrieving through the MARS client protocol
-------------------------------------------

``MarsRequestHandle`` wraps ``metkit::mars::MarsRequestHandle``, an
``eckit::DataHandle`` that sends a request to a MARS server and reads the
fields back. It takes an expanded request and the database configuration the
C++ constructor takes. Opening it connects and performs the handshake, and
returns a ``DataHandle<Reading>`` with the estimated length of the data:

.. literalinclude:: examples/hermes.rs
   :language: rust
   :start-after: // [hermes]
   :end-before: // [end hermes]

Because the open handle is an ``eckit::DataHandle``, it can be passed to
``eckit::MessageReader`` to iterate the fields instead of reading the bytes.

Errors
------

``metkit::Error`` has three variants: ``Metkit`` for metkit's own exception
classes, ``Eckit`` for eckit's, and ``Other`` for anything else. Expansion
failures are ``eckit::UserError``:

.. literalinclude:: examples/errors.rs
   :language: rust
   :start-after: // [errors]
   :end-before: // [end errors]
   :dedent:

Naming the inner variants needs ``eckit`` and ``metkit-sys`` as dependencies
of the application. ``Error`` implements ``std::error::Error``, so ``?`` and
``Box<dyn Error>`` work without them.

Threads
-------

.. list-table::
   :widths: 30 20 50
   :header-rows: 1

   * - Type
     - Markers
     - Reason
   * - ``MarsRequest``
     - ``Send + Sync``
     - The C++ object holds its parameters by value, and the ``const``
       methods do not mutate
   * - ``HyperCube``
     - ``Send + Sync``
     - As above
   * - ``CodesHandle``
     - ``Send``
     - ecCodes caches decoded state inside a handle on read, so two threads
       must not share one; move it, or put it behind a ``Mutex``
   * - ``ParsedRequests``
     - neither
     - Collect into a ``Vec<MarsRequest>`` with ``to_vec`` before handing
       the requests to other threads
