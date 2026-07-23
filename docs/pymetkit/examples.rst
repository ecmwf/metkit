Examples
========

The examples below use ``PyMetKit``'s public API and are executed as sybil tests.
The building and accessing examples are self-contained; the expanding, equality,
merging and parsing examples require the MARS language definitions shipped with
``metkit`` (see :doc:`installation`).

Building a request
------------------

A :class:`~pymetkit.pymetkit.MarsRequest` is a verb plus a selection of parameters
passed as a plain mapping. Values may be scalars, numbers, collections, or
``/``-separated strings.

.. code-block:: python

   from pymetkit import MarsRequest
   request = MarsRequest("retrieve", {"class": "od", "param": [151, 129]})
   assert request.verb() == "retrieve"
   assert request["class"] == "od"
   assert request["param"] == ['151', '129']

Numbers, ranges and ``/``-separated strings are normalized to lists of strings:

.. code-block:: python

   request = MarsRequest("retrieve", {"step": range(0, 13, 6), "date": "20200101/20200102"})
   assert request["step"] == ["0", "6", "12"]
   assert request["date"] == ["20200101", "20200102"]

Accessing values
----------------

A request behaves like a read/write mapping. A parameter with a single value returns a
scalar, one with several values returns a list:

.. code-block:: python

   request = MarsRequest("retrieve", {"class": "od", "param": [151, 129]})
   assert "class" in request
   assert sorted(request.keys()) == ["class", "param"]
   assert request.num_values("param") == 2
   request["expver"] = "0001"
   assert request["expver"] == "0001"

Iterating over a request yields ``(name, value)`` pairs, mirroring a plain dict:

.. code-block:: python

   request = MarsRequest("retrieve", {"class": "od", "param": [151, 129]})
   assert list(request) == [("class", "od"), ("param", ["151", "129"])]

Pass a request to :func:`dict` to get a plain Python mapping, useful for
serialisation or inspection:

.. code-block:: python

   request = MarsRequest("retrieve", {"class": "od", "param": [151, 129]})
   assert dict(request) == {"class": "od", "param": ["151", "129"]}

Manipulating a request
----------------------

Parameters can be added or overwritten after construction. All value forms accepted
at construction time are also valid for assignment — scalars, integers, ranges, lists
and ``/``-separated strings are all normalised the same way:

.. code-block:: python

   from pymetkit import MarsRequest

   request = MarsRequest("retrieve", {"class": "od", "expver": "0001"})

   request["date"] = "20230101/20230102"  # slash-separated → list
   request["step"] = range(0, 13, 6)      # range → list
   request["param"] = 130                 # integer → scalar

   assert request["date"] == ["20230101", "20230102"]
   assert request["step"] == ["0", "6", "12"]
   assert request["param"] == "130"

   request["step"] = [0, 6]              # overwrite with a shorter list
   assert request["step"] == ["0", "6"]

A new request can be derived from an existing one by snapshotting it with
:func:`dict`, adjusting selected values, and constructing a fresh request:

.. code-block:: python

   from pymetkit import MarsRequest

   base = MarsRequest("retrieve", {"class": "od", "date": "-1", "param": "130", "step": "0"})
   derived = MarsRequest(base.verb(), {**dict(base), "date": "20230101"})

   assert derived["date"] == "20230101"
   assert derived["param"] == "130"

Use ``in`` to guard access to parameters that may not be present:

.. code-block:: python

   from pymetkit import MarsRequest

   request = MarsRequest("retrieve", {"class": "od", "param": "130"})
   step = request["step"] if "step" in request else "0"
   assert step == "0"

Equality and hashing
--------------------

Two requests are equal when they expand to the same result. The MARS language
defines aliases, so ``"od"`` and ``"operational"`` for ``class`` refer to the same
dataset — equality reflects that:

.. code-block:: python

   from pymetkit import MarsRequest

   r1 = MarsRequest("retrieve", {"class": "od",          "date": "20230101", "param": "130"})
   r2 = MarsRequest("retrieve", {"class": "operations",  "date": "20230101", "param": "130"})

   assert r1 == r2

Expanding and validating
-------------------------

:meth:`~pymetkit.pymetkit.MarsRequest.expand` returns a new request expanded against the
MARS language definition; :meth:`~pymetkit.pymetkit.MarsRequest.validate` checks a request
without inheriting defaults and raises :class:`~pymetkit.MetKitException` on invalid input.

.. code-block:: python

   from pymetkit import MarsRequest

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

   expanded = request.expand()          # inherit=True: fills in default values
   assert expanded.verb() == "retrieve"

   request.validate()                   # raises MetKitException if invalid

Merging requests
----------------

:meth:`~pymetkit.pymetkit.MarsRequest.merge` combines the values of two requests that
carry the same parameters, keeping ``self``'s values first and appending only the
values of ``other`` that are not already present:

.. code-block:: python

   from pymetkit import MarsRequest

   left = MarsRequest("retrieve", {"class": "od", "date": "-1", "levtype": "sfc"})
   right = MarsRequest("retrieve", {"class": "od", "date": "20230101", "levtype": "sfc"})

   merged = left.merge(right)
   assert merged["date"] == ["-1", "20230101"]

Parsing requests
----------------

:func:`~pymetkit.pymetkit.parse_mars_request` parses one or more requests from a string
or a file-like object:

.. code-block:: python

   from pymetkit import parse_mars_request

   requests = parse_mars_request("retrieve,class=od,date=-1,param=129,step=12")
   assert len(requests) == 1
   assert requests[0].verb() == "retrieve"
