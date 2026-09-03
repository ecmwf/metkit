.. _PyMetkit_Introduction:

pymetkit
========

:Version: |version|

.. warning::
   These documentation pages are a work in progress.

``pymetkit`` is the Python interface to :ref:`Metkit <Metkit_Introduction>`.
It provides a thin, idiomatic Python layer over the Metkit library installed on
your system, so you can parse and manipulate MARS requests directly from Python
scripts and notebooks.

It exposes the MARS request model as Pythonic objects built on a
`pybind11 <https://github.com/pybind/pybind11>`__ extension module that binds the
``metkit`` C++ library directly. The interface is organized in three layers:

- :doc:`api` — the Pythonic layer: :class:`~pymetkit.pymetkit.MarsRequest`
  (a verb plus a :data:`~pymetkit.pymetkit_type.MarsSelection`),
  :func:`~pymetkit.pymetkit.parse_mars_request`, and
  :func:`~pymetkit.pymetkit_batch.expand`.
- ``pymetkit._internal`` — locates and loads ``libmetkit`` at import time via
  `findlibs <https://github.com/ecmwf/findlibs>`__, verifies the runtime version
  against the build-time version, initialises the bindings, and re-exports the
  low-level symbols. Internal; not part of the public API.
- :doc:`bindings` — the compiled ``pymetkit_bindings`` module that binds
  ``metkit::mars::MarsRequest`` and ``metkit::mars::MarsExpansion``.

.. note::

   ``pymetkit`` supersedes the legacy CFFI-based interface. If you are migrating from
   the old package, see :doc:`legacy`.

.. toctree::
   :maxdepth: 2
   :caption: Contents:
   :hidden:

   installation
   examples
   api

.. toctree::
   :maxdepth: 2
   :caption: Technical Insights:
   :hidden:

   bindings
   development
   legacy

Quick start
-----------

.. code-block:: python

   from pymetkit import MarsRequest, expand, parse_mars_request

   request = MarsRequest(
       "retrieve",
       {
           "class": "od",
           "domain": "g",
           "date": "-1",
           "expver": "0001",
           "step": range(0, 13, 6),
       },
   )

   # Expand a single request.
   expanded = expand(request)
   print(expanded.verb(), dict(expanded))

   # Expand a batch. Internal language checks are performed once for the
   # whole batch, rather than once per request.
   requests = [
       MarsRequest("retrieve", {"class": "od", "date": "-1", "param": str(p)})
       for p in [129, 130, 131]
   ]
   expanded_batch = expand(requests)

   for req in parse_mars_request("retrieve,class=od,date=-1,param=129,step=12"):
       print(req.verb(), req["param"])
