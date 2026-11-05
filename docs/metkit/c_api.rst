.. _Metkit_C_API:

C API
=====

Metkit also exposes a plain C interface, declared in ``metkit_c.h``. It is a
thin wrapper over the C++ library and is the foundation the Python bindings
(:ref:`pymetkit <PyMetkit_Introduction>`) build upon.

All functions return a :c:enum:`metkit_error_values_t` (with iterator stepping
reported via :c:enum:`metkit_iterator_status_t`); on failure, a human-readable
message can be retrieved with :c:func:`metkit_get_error_string`. Opaque request
and iterator types are created and released through their matching
``_new``/``_delete`` functions.

.. doxygenfile:: metkit_c.h
   :project: Metkit
