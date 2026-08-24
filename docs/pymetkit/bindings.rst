pybind11 bindings
=================

``pymetkit_bindings`` is the compiled `pybind11 <https://github.com/pybind/pybind11>`__
extension module that binds the ``metkit`` C++ classes directly. It is the lowest layer
of ``PyMetKit`` and is consumed by :mod:`pymetkit._internal`; the raw ``MarsRequest``
class it exposes is re-exported there as ``_MarsRequest``.

.. warning::

   This is an internal, low-level layer. Application code should use the Pythonic
   :doc:`api` (:class:`pymetkit.pymetkit.MarsRequest`,
   :data:`pymetkit.pymetkit_type.MarsSelection` and
   :func:`pymetkit.pymetkit.parse_mars_request`) rather than these bindings directly.
   The signatures below map one-to-one onto ``metkit::mars::MarsRequest`` and are not
   covered by the same value-normalization or error-translation guarantees.

The extension is built from ``src/pymetkit_bindings/bindings.cc`` and staged into the
wheel as its own top-level package. The shared library ``libmetkit`` and its
dependencies must be loaded (via ``findlibs``) before the module is imported, and
:func:`init_bindings` must be called once before any other call — both are handled
automatically by :mod:`pymetkit._internal`.

Module-level functions
----------------------

.. py:module:: pymetkit_bindings

.. py:function:: init_bindings()

   Initialise the ``eckit`` runtime (``eckit::Main``). Must be called once, before any
   other binding call, when ``metkit`` is loaded as a shared library from Python.

.. py:function:: version_info()

   Return a list of ``(name, version, git_sha1, path)`` tuples describing every
   ``eckit``-registered library currently loaded (e.g. ``eckit``, ``eccodes``,
   ``metkit``). Used by the ``python -m pymetkit`` diagnostics CLI.

   :rtype: list[tuple[str, str, str, str]]

.. py:function:: parse_marsrequest(string, strict)

   Parse a single MARS request from ``string``.

   :param str string: the MARS request text.
   :param bool strict: raise an error (rather than a warning) on invalid values.
   :returns: the parsed request.
   :rtype: MarsRequest

.. py:function:: parse_marsrequests(string, strict)

   Parse one or more MARS requests from ``string``.

   :param str string: text containing one or more MARS requests.
   :param bool strict: raise an error (rather than a warning) on invalid values.
   :returns: the parsed requests.
   :rtype: list[MarsRequest]

Classes
-------

.. py:class:: MarsRequest

   A thin binding of ``metkit::mars::MarsRequest``. Values are passed and returned as
   lists of strings; no normalization is performed at this layer.

   .. py:method:: __init__()
                  __init__(verb)

      Construct an empty request, or a request with the given ``verb``.

      :param str verb: the request verb (e.g. ``"retrieve"``).

   .. py:method:: verb()

      Return the request verb.

      :rtype: str

   .. py:method:: set_verb(verb)

      Set the request verb.

      :param str verb: the verb to set.

   .. py:method:: set(param, values)

      Set the values of a parameter.

      :param str param: the parameter name.
      :param list[str] values: the parameter values.

   .. py:method:: has(param)

      Return whether ``param`` is present in the request.

      :param str param: the parameter name.
      :rtype: bool

   .. py:method:: params()

      Return the parameter names present in the request.

      :rtype: list[str]

   .. py:method:: count_values(param)

      Return the number of values held for ``param``.

      :param str param: the parameter name.
      :rtype: int

   .. py:method:: values(param)

      Return the values held for ``param``.

      :param str param: the parameter name.
      :rtype: list[str]

   .. py:method:: merge(other)

      Merge ``other`` into this request in place: for each shared parameter, the values
      of ``other`` that are not already present are appended (order-preserving union).

      :param MarsRequest other: the request to merge in.

   .. py:method:: expand(inherit, strict)

      Return the request expanded against the MARS language definition, using
      ``metkit::mars::MarsExpansion``.

      :param bool inherit: populate the expanded request with default values.
      :param bool strict: raise an error (rather than a warning) on invalid values.
      :returns: the expanded request.
      :rtype: MarsRequest

   .. py:method:: __repr__()

      Return the request rendered as a MARS request string (``asString()``).

      :rtype: str
