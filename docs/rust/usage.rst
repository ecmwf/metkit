Using the ``metkit`` Crate
==========================

The code on this page is quoted from the examples of the ``metkit`` crate;
:ref:`rust/building:Running the Examples` says how to run them. The methods
of each type are listed on `docs.rs <https://docs.rs/metkit>`_.

Runtime Initialisation
----------------------

Call ``eckit::init()`` before any other call. Metkit's log output then goes
through the ``log`` crate under the target ``metkit``, so an ``env_logger``
filter of ``RUST_LOG=metkit=debug`` switches on its debug channel.

A program that retrieves data through the MARS client protocol also sets the
client metadata sent with every request:

.. literalinclude:: ../../rust/crates/metkit/examples/hermes.rs
   :language: rust
   :start-after: // [init]
   :end-before: // [end init]
   :dedent:

``metkit::environment::environment_request()`` returns that metadata as a
``MarsRequest``.

Building Requests
-----------------

A new request is ``MarsRequest<Raw>``, the state that can be edited:

.. literalinclude:: ../../rust/crates/metkit/examples/requests.rs
   :language: rust
   :start-after: // [build]
   :end-before: // [end build]
   :dedent:

``set`` accepts these value types. The last write to a key wins:

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
exactly one and fails with ``eckit::UserError`` when the key has several;
``values`` returns all of them. ``params`` lists the keywords and ``count``
is the number of fields the request describes. ``Display`` prints the same
text as the C++ ``dump``; ``to_json`` prints the parameters as a JSON object
without the verb:

.. literalinclude:: ../../rust/crates/metkit/examples/requests.rs
   :language: rust
   :start-after: // [read]
   :end-before: // [end read]
   :dedent:

Expansion
---------

``expand`` runs the MARS language over the request: aliases become
canonical values (``t`` becomes ``130``), relative dates are resolved, ranges
such as ``0/to/12/by/6`` are enumerated and keywords are reordered into the
language's axis order.

.. literalinclude:: ../../rust/crates/metkit/examples/requests.rs
   :language: rust
   :start-after: // [expand]
   :end-before: // [end expand]
   :dedent:

``expand`` takes two flags:

``inherit``
   When set, keywords the request does not mention are filled with the
   defaults of the language. A ``retrieve`` with only ``class=od`` and a date
   gains ``type=an``, ``stream=oper``, ``levtype=pl``, ``time=1200``,
   ``step=0``, ``expver=0001``, ``domain=g``, six pressure levels and
   ``param=129``. When unset, the request keeps only the keywords it had.

``strict``
   When set, it is an error for the request to hold a keyword the language
   rules out in the context of the others or to lack one those rules
   require. When unset, the keyword is removed or filled in silently.

Matching a value against the language is exact by default. With the
environment variable ``METKIT_LANGUAGE_STRICT_MODE=false`` a value also
matches an entry it abbreviates.

Parsing MARS Text
-----------------

``metkit::parse`` reads text holding one or more requests, expands each and
returns ``ParsedRequests<Expanded>``:

.. literalinclude:: ../../rust/crates/metkit/examples/requests.rs
   :language: rust
   :start-after: // [parse]
   :end-before: // [end parse]
   :dedent:

``str::parse`` does the same for text that holds exactly one request. The
``strict`` argument of ``parse`` is the ``strict`` flag of expansion.

``metkit::tokenize`` runs the parser alone, with no expansion and no check
of the verb, so it accepts ``environ`` and other verbs the language does not
define. Its results are ``MarsRequest<Raw>``:

.. literalinclude:: ../../rust/crates/metkit/examples/requests.rs
   :language: rust
   :start-after: // [tokenize]
   :end-before: // [end tokenize]
   :dedent:

Trusted Input
-------------

Text that is canonical already, for example a request a server produced, can
skip expansion with ``from_trusted``. It is parsed and tagged ``Expanded``
without a check; a value that is not canonical is detected only when the
request is used. ``set_trusted`` edits an expanded request in
place under the same contract:

.. literalinclude:: ../../rust/crates/metkit/examples/requests.rs
   :language: rust
   :start-after: // [trusted]
   :end-before: // [end trusted]
   :dedent:

The checked way to edit an expanded request is to re-tag it as raw with
``into_raw``, edit it and expand it again:

.. literalinclude:: ../../rust/crates/metkit/examples/requests.rs
   :language: rust
   :start-after: // [edit]
   :end-before: // [end edit]
   :dedent:

HyperCube
---------

``HyperCube`` is built from an expanded request and enumerates the fields
the request describes, one cell per combination of values, keeping track of
which cells have been seen. ``clear`` marks a cell and returns whether it was
still vacant; ``field_ordinal`` gives the position of a field in the
request's order:

.. literalinclude:: ../../rust/crates/metkit/examples/hypercube.rs
   :language: rust
   :start-after: // [cube]
   :end-before: // [end cube]
   :dedent:

``contains``, ``clear`` and ``field_ordinal`` take a request in either
state, since a request built from a GRIB message is ``Raw``.

GRIB Keys
---------

``CodesHandle`` holds one GRIB message in ecCodes and needs the ``grib``
feature, on by default. A handle comes from a file (``from_file``,
``from_file_at_offset``), from bytes (``from_message``, which copies them) or
from an ecCodes sample (``from_sample``). ``get`` returns ``String``, ``i64``, ``f64``, ``Vec<f64>`` or ``Vec<i64>``;
``set`` takes ``&str``, ``String``, ``i64``, ``f64`` or ``&[f64]``. The type
selects the ecCodes accessor of the same kind:

.. literalinclude:: ../../rust/crates/metkit/examples/codes.rs
   :language: rust
   :start-after: // [codes]
   :end-before: // [end codes]
   :dedent:

``message_data`` borrows the encoded message from the handle; copy it or
build a second handle with ``try_clone`` to keep it past the handle's life.
``is_defined`` is true for a key the message has, ``is_missing`` for a key
coded as missing and ``has`` for a key that is defined and not missing;
``set_missing`` codes a key as missing.

Messages
--------

A request can be built from the metadata of a GRIB message that
``eckit::MessageReader`` found in a data handle. The result is
``MarsRequest<Raw>``, since GRIB keys are not language-expanded:

.. literalinclude:: ../../rust/crates/metkit/examples/codes.rs
   :language: rust
   :start-after: // [messages]
   :end-before: // [end messages]
   :dedent:

The reader finds GRIB messages only in a program built with the ``grib``
feature.

Streams
-------

``encode`` and ``decode`` move a request through an ``eckit::Stream`` in the
format of the C++ library, so a request written in Rust is read by C++ and
the other way round. ``MarsRequest`` also implements ``eckit::StreamWrite``.
A decoded request is ``Raw``, since the wire carries no record of expansion:

.. literalinclude:: ../../rust/crates/metkit/examples/stream.rs
   :language: rust
   :start-after: // [stream]
   :end-before: // [end stream]
   :dedent:

MARS Retrieval
--------------

``MarsRequestHandle`` is an ``eckit::DataHandle`` that sends an expanded
request to a MARS server and reads the fields back. Opening it sends the
request and returns once the server has answered with the size of the data
it will send:

.. literalinclude:: ../../rust/crates/metkit/examples/hermes.rs
   :language: rust
   :start-after: // [hermes]
   :end-before: // [end hermes]

``eckit::MessageReader`` iterates the fields of the open handle.
``MarsRequestHandle`` converts into ``eckit::DataHandle<Closed>``. The
``hermes`` example takes the path of a database configuration and needs a
reachable server.

Errors
------

``metkit::Error`` has one variant per metkit exception class,
``CodesException`` and ``CodesWrongLength``, an ``Eckit`` variant holding an
``eckit::Error`` and ``Other`` for anything else. Expansion failures are
``eckit::UserError``; reading a GRIB key that does not exist is a
``CodesException``:

.. literalinclude:: ../../rust/crates/metkit/examples/errors.rs
   :language: rust
   :start-after: // [errors]
   :end-before: // [end errors]
   :dedent:

The ``Display`` output of ``metkit::Error`` keeps the class name, for
example ``metkit::CodesException: ...``.

Threads
-------

``MarsRequest`` and ``HyperCube`` are ``Send`` and ``Sync``.

``CodesHandle`` and ``MarsRequestHandle`` are ``Send`` but not ``Sync``.

``ParsedRequests`` is neither; collect it with ``to_vec`` before handing the
requests to other threads.
