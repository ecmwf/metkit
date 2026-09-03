# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Performance stress test for the MarsRequest Python layer.

Not registered with ctest. Run it deliberately, with ``-s`` to see the report:

    export PYTHONPATH=<build>/pymetkit-python-package-staging
    pytest tests/pymetkit/performance/ -s

The expansion-backed cases additionally need the MARS language definitions that
ctest normally injects; without them those cases skip:

    export METKIT_HOME=<build>
    export ECCODES_DEFINITION_PATH=<build>/share/eccodes/definitions

Two costs are easy to incur without noticing, and this test makes both visible:

- ``__getitem__`` and ``__iter__`` copy the value list on *every* read.
- ``__eq__`` expands both sides, so comparing requests drives two C++ expansions.
"""

from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager
from time import perf_counter

import pytest

from pymetkit import MarsRequest, expand

# -- Workload scale ---------------------------------------------------------

REQUEST_COUNT = 1000
WIDE_PARAM_COUNT = 50
LARGE_VALUE_COUNT = 500
ACCESS_REPEATS = 5
COMPARE_COUNT = 100
MERGE_COUNT = 100

# -- Time budgets, in seconds ----------------------------------------------
# Deliberately ~50x above expected runtime: these catch order-of-magnitude
# regressions, not small drifts. Retune here.

BUDGET_CONSTRUCT = 3.0
BUDGET_CONSTRUCT_WIDE = 5.0
BUDGET_CONSTRUCT_LARGE_VALUES = 5.0
BUDGET_READ = 5.0
BUDGET_WRITE = 5.0
BUDGET_CHEAP_QUERY = 3.0
BUDGET_ITERATE = 3.0
BUDGET_DICT = 3.0
BUDGET_REPR = 10.0
BUDGET_EXPAND_SINGLE = 60.0
BUDGET_EXPAND_BATCH = 5.0
BUDGET_EXPAND_PARALLEL = 3.0
PARALLEL_WORKER_COUNT = 20
BUDGET_COMPARE = 15.0
BUDGET_MERGE = 30.0


@contextmanager
def timed(label, budget_seconds, operations):
    """Print the measurement, then fail if it exceeded its budget."""
    start = perf_counter()
    yield
    elapsed = perf_counter() - start

    rate = operations / elapsed if elapsed > 0 else float("inf")
    print(f"{label:<38} {elapsed:8.3f}s {rate:12,.0f} ops/s")

    assert elapsed < budget_seconds, f"{label} took {elapsed:.3f}s, budget {budget_seconds:.1f}s"


def _selection(index):
    """A realistic selection. `param` varies so requests stay distinct."""
    return {
        "class": "od",
        "domain": "g",
        "date": "-1",
        "expver": "0001",
        "param": str(130 + index % 20),
        "step": range(0, 13, 6),
    }


def _requests(count=REQUEST_COUNT):
    return [MarsRequest("retrieve", _selection(index)) for index in range(count)]


@pytest.fixture(scope="module")
def expandable_requests():
    """Requests for the expansion tier; skips when the language defs are absent."""
    try:
        expand(MarsRequest("retrieve", _selection(0)))
    except Exception as error:
        pytest.skip(f"MARS language definitions unavailable: {error}")

    return _requests()


@pytest.fixture(scope="module")
def expandable_requests_small():
    """Requests for the expansion tier; skips when the language defs are absent."""
    try:
        expand(MarsRequest("retrieve", _selection(0)))
    except Exception as error:
        pytest.skip(f"MARS language definitions unavailable: {error}")

    return _requests(100)


# Pure Python layer: construction
def test_construct_requests():
    with timed("construct", BUDGET_CONSTRUCT, REQUEST_COUNT):
        _requests()


def test_construct_wide_selections():
    # Normalization runs per key, so widen the selection to scale that cost.
    wide = {f"key_{index:03d}": str(index) for index in range(WIDE_PARAM_COUNT)}

    with timed("construct wide", BUDGET_CONSTRUCT_WIDE, REQUEST_COUNT * WIDE_PARAM_COUNT):
        for _ in range(REQUEST_COUNT):
            MarsRequest("retrieve", wide)


def test_construct_large_value_lists():
    # Two shapes that both fan out to LARGE_VALUE_COUNT strings: a list of ints
    # (stringified element-wise) and a '/'-separated string (split).
    as_list = list(range(LARGE_VALUE_COUNT))
    as_range_expression = "/".join(str(value) for value in range(LARGE_VALUE_COUNT))

    with timed("construct large values", BUDGET_CONSTRUCT_LARGE_VALUES, REQUEST_COUNT * 2):
        for _ in range(REQUEST_COUNT):
            MarsRequest("retrieve", {"step": as_list})
            MarsRequest("retrieve", {"step": as_range_expression})


# Pure Python layer: accessors
def test_getitem_churn():
    requests = _requests()
    keys = list(requests[0].keys())
    reads = REQUEST_COUNT * len(keys) * ACCESS_REPEATS

    # Every read rebuilds the value list, so this is the copy cost, not a lookup.
    with timed("__getitem__", BUDGET_READ, reads):
        for _ in range(ACCESS_REPEATS):
            for request in requests:
                for key in keys:
                    request[key]


def test_setitem_churn():
    requests = _requests()
    writes = REQUEST_COUNT * ACCESS_REPEATS

    with timed("__setitem__", BUDGET_WRITE, writes):
        for repeat in range(ACCESS_REPEATS):
            for request in requests:
                request["step"] = range(0, 13, 6)
                request["param"] = 130 + repeat


def test_cheap_queries_stay_cheap():
    # Baseline: these touch the internal dict directly. Compare against
    # __getitem__ above to see what the value-copy actually costs.
    requests = _requests()
    keys = list(requests[0].keys())
    operations = REQUEST_COUNT * len(keys) * 3

    with timed("keys/contains/num_values", BUDGET_CHEAP_QUERY, operations):
        for request in requests:
            list(request.keys())
            for key in keys:
                key in request
                request.num_values(key)


def test_iteration():
    requests = _requests()
    keys = len(list(requests[0].keys()))

    with timed("iterate pairs", BUDGET_ITERATE, REQUEST_COUNT * keys):
        for request in requests:
            for _key, _value in request:
                pass


def test_dict_conversion():
    requests = _requests()

    with timed("dict()", BUDGET_DICT, REQUEST_COUNT):
        for request in requests:
            dict(request)


def test_repr_marshalling():
    # repr goes through _to_internal(), so this is the Python -> C++ boundary
    # cost without the language engine on top.
    requests = _requests()

    with timed("repr()", BUDGET_REPR, REQUEST_COUNT):
        for request in requests:
            repr(request)


# Expansion-backed: needs the MARS language definitions
def test_expand_single(expandable_requests_small):
    # Baseline: one MarsExpansion instance per request.
    with timed(
        f"expand() {len(expandable_requests_small)} single", BUDGET_EXPAND_SINGLE, len(expandable_requests_small)
    ):
        for request in expandable_requests_small:
            request.expand()


def test_expand_batch(expandable_requests):
    # Batch: one MarsExpansion instance shared across all requests.
    with timed(f"expand() {len(expandable_requests)} batch", BUDGET_EXPAND_BATCH, REQUEST_COUNT):
        expand(expandable_requests)


def test_expand_parallel_batches(expandable_requests):
    # Parallel: GIL released during C++ expansion, so threads run concurrently.
    # Each worker gets an equal sub-batch; MarsExpansion instances are independent.
    batch_size = len(expandable_requests) // PARALLEL_WORKER_COUNT
    batches = [expandable_requests[i * batch_size : (i + 1) * batch_size] for i in range(PARALLEL_WORKER_COUNT)]

    with timed(f"expand() {PARALLEL_WORKER_COUNT}x parallel batch", BUDGET_EXPAND_PARALLEL, REQUEST_COUNT):
        with ThreadPoolExecutor(max_workers=PARALLEL_WORKER_COUNT) as pool:
            futures = [pool.submit(expand, batch) for batch in batches]
            results = [future.result() for future in futures]

    assert sum(len(batch) for batch in results) == REQUEST_COUNT


def test_equality(expandable_requests):
    # Two expansions per comparison (one per side).
    left = expandable_requests[:COMPARE_COUNT]
    right = _requests(COMPARE_COUNT)

    with timed("__eq__", BUDGET_COMPARE, COMPARE_COUNT):
        for request_a, request_b in zip(left, right):
            request_a == request_b


def test_merge(expandable_requests):
    # merge() validates the result, which is a further expansion.
    left = expandable_requests[:MERGE_COUNT]
    right = _requests(MERGE_COUNT)

    with timed("merge()", BUDGET_MERGE, MERGE_COUNT):
        for request_a, request_b in zip(left, right):
            request_a.merge(request_b)
