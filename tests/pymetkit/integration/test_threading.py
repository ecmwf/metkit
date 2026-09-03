# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Thread-safety tests for concurrent MARS request expansion.

Each thread creates its own MarsExpansion instance; the only shared state is
the static YAML tables loaded once via pthread_once. The GIL is released for
the duration of the C++ expansion call, so threads genuinely run in parallel.
"""

from concurrent.futures import ThreadPoolExecutor, as_completed

import pytest

from pymetkit import MarsRequest, expand


def _make_requests(count):
    return [MarsRequest("retrieve", {"class": "od", "date": "-1", "param": str(130 + i % 20)}) for i in range(count)]


def _try_expand_one(request):
    """Used as the probe in the expandable fixture."""
    expand(request)


# Fixtures
@pytest.fixture(scope="module")
def base_requests():
    return _make_requests(40)


@pytest.fixture(scope="module")
def expanded_reference(base_requests):
    """Sequential reference expansion; skips when language defs are absent."""
    try:
        return expand(base_requests)
    except Exception as error:
        pytest.skip(f"MARS language definitions unavailable: {error}")


# Tests
def test_concurrent_expand_batches(base_requests, expanded_reference):
    """Split requests into 4 batches and expand each in a separate thread."""
    n_workers = 10
    batch_size = len(base_requests) // n_workers

    # Split into equal-sized batches.
    batches = [base_requests[i * batch_size : (i + 1) * batch_size] for i in range(n_workers)]

    results = []
    with ThreadPoolExecutor(max_workers=n_workers) as pool:
        futures = {pool.submit(expand, batch): i for i, batch in enumerate(batches)}
        # Collect in submission order so we can compare against the reference.
        ordered = [None] * n_workers
        for future in as_completed(futures):
            ordered[futures[future]] = future.result()

    results = [req for batch in ordered for req in batch]

    assert len(results) == len(expanded_reference)
    for got, expected in zip(results, expanded_reference):
        assert got.verb() == expected.verb()
        assert got["date"] == expected["date"]
        assert got["class"] == expected["class"]


def test_concurrent_expand_singles(base_requests, expanded_reference):
    """Expand each request individually in its own thread."""
    with ThreadPoolExecutor(max_workers=20) as pool:
        futures = [pool.submit(expand, req) for req in base_requests]
        results = [f.result() for f in futures]

    assert len(results) == len(expanded_reference)
    for got, expected in zip(results, expanded_reference):
        assert got.verb() == expected.verb()
        assert got["date"] == expected["date"]


def test_concurrent_no_cross_contamination(base_requests):
    """Requests expanded in parallel must match those expanded sequentially.

    Catches races where threads corrupt each other's MarsLanguage cache.
    """
    try:
        sequential = expand(base_requests)
    except Exception as error:
        pytest.skip(f"MARS language definitions unavailable: {error}")

    with ThreadPoolExecutor(max_workers=4) as pool:
        futures = [pool.submit(expand, req) for req in base_requests]
        parallel = [f.result() for f in futures]

    for got, expected in zip(parallel, sequential):
        assert repr(got) == repr(expected)
