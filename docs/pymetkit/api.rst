API
===

The ``PyMetKit`` API provides a Pythonic interface to ``metkit``'s MARS request
model. A :class:`~pymetkit.pymetkit.MarsRequest` is a verb together with a
:data:`~pymetkit.pymetkit_type.MarsSelection` — a type alias for a user-supplied
key-value mapping. Values are normalised at construction time (scalars wrapped,
collections stringified) and stored directly in the underlying C++ object.
Operations that require the MARS language engine (expansion, validation, equality,
merging and parsing) are delegated to ``metkit`` through the :doc:`bindings` layer.

MarsRequest
-----------
.. autoapiclass:: pymetkit.pymetkit.MarsRequest
   :members:

Expansion
---------
.. autoapifunction:: pymetkit.pymetkit_batch.expand

Parsing
-------
.. autoapifunction:: pymetkit.pymetkit_batch.parse_mars_request

MarsSelection
-------------
.. autoapidata:: pymetkit.pymetkit_type.MarsSelection

Exceptions
----------
.. py:exception:: pymetkit.MetKitException

   Raised when the underlying ``metkit`` library reports an error, for example when
   :func:`~pymetkit.pymetkit_batch.expand` or
   :meth:`~pymetkit.pymetkit.MarsRequest.validate` encounters a request that is
   incompatible with the MARS language definition. Subclasses :class:`RuntimeError`.
