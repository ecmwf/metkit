.. _Metkit_CPP_API:

C++ API
=======

Metkit is primarily a C++ library. This page documents the main public C++
interfaces.

The MarsRequest class
---------------------

:cpp:class:`metkit::mars::MarsRequest` is the central data structure of the
library: it represents a single MARS request (a verb plus a set of keyword
parameters and their values) that can be parsed, inspected and manipulated.

.. doxygenclass:: metkit::mars::MarsRequest
   :members:
   :undoc-members:
