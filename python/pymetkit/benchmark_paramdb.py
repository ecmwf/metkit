#!/usr/bin/env python3
"""
benchmark_paramdb.py  —  ParamDB performance benchmark
=======================================================

Measures:
  1. Construction time  (lazy — no I/O happens at init)
  2. Cold-load time     (first lookup, which triggers the YAML/JSON/network load)
  3. Warm-lookup time   (all public methods on an already-loaded database)

Online vs offline comparison
-----------------------------
Offline mode reads from the bundled ``parameter_metadata.yaml``.
Online mode can read from:
  - A local JSON cache (fast, no network needed)
  - The live ECMWF API (slow, needs internet; triggered when cache is absent)

Usage
-----
    # Offline only (default, no network needed):
    python benchmark_paramdb.py

    # Include online benchmarks (fetches from API if no local cache):
    python benchmark_paramdb.py --online

    # Use a custom cache directory:
    python benchmark_paramdb.py --online --cache-path /tmp/paramdb_bench

    # Change the number of warm-lookup iterations:
    python benchmark_paramdb.py --iterations 1000

    # Suppress ANSI colours:
    python benchmark_paramdb.py --no-color
"""

import argparse
import gc
import statistics
import sys
import tempfile
import time
from pathlib import Path

# ---------------------------------------------------------------------------
# Path setup: works both from an editable install and directly from the repo
# ---------------------------------------------------------------------------

_here = Path(__file__).resolve().parent
_src = _here / "src"
if _src.exists():
    sys.path.insert(0, str(_src))

try:
    from pymetkit import ParamDB  # noqa: E402
except (ImportError, AttributeError) as _err:
    print(
        "ERROR: Could not import pymetkit.ParamDB.\n"
        f"       {_err}\n\n"
        "       Make sure the metkit shared library is on your library path.\n"
        "       On macOS:  export DYLD_LIBRARY_PATH=/path/to/metkit/lib\n"
        "       On Linux:  export LD_LIBRARY_PATH=/path/to/metkit/lib\n"
    )
    sys.exit(1)

# ---------------------------------------------------------------------------
# ANSI colour helpers
# ---------------------------------------------------------------------------

_USE_COLOR = True


def _c(text: str, code: str) -> str:
    return f"\033[{code}m{text}\033[0m" if _USE_COLOR else text


def bold(t: str)   -> str: return _c(t, "1")
def dim(t: str)    -> str: return _c(t, "2")
def green(t: str)  -> str: return _c(t, "32")
def yellow(t: str) -> str: return _c(t, "33")
def cyan(t: str)   -> str: return _c(t, "36")
def red(t: str)    -> str: return _c(t, "31")

# ---------------------------------------------------------------------------
# Timing primitives
# ---------------------------------------------------------------------------

_NS_PER_US = 1_000
_NS_PER_MS = 1_000_000


def _bench(fn, n: int = 1) -> list[float]:
    """Run *fn* *n* times; return elapsed times in **milliseconds**."""
    results: list[float] = []
    for _ in range(n):
        gc.collect()
        t0 = time.perf_counter_ns()
        fn()
        results.append((time.perf_counter_ns() - t0) / _NS_PER_MS)
    return results


def _fmt(ms_list: list[float]) -> str:
    """Format timing samples as 'mean ± std' in the most readable unit."""
    mean = statistics.mean(ms_list)
    std  = statistics.stdev(ms_list) if len(ms_list) > 1 else 0.0
    if mean < 0.01:                        # sub-10 µs → show in µs
        return f"{mean*1000:.2f} µs  ± {std*1000:.2f} µs"
    if mean < 1.0:                         # sub-ms  → show in µs
        return f"{mean*1000:.1f} µs  ± {std*1000:.1f} µs"
    return f"{mean:.2f} ms  ± {std:.2f} ms"


def _mean_ms(ms_list: list[float]) -> float:
    return statistics.mean(ms_list)

# ---------------------------------------------------------------------------
# Layout helpers
# ---------------------------------------------------------------------------

_W_LABEL  = 48
_W_RESULT = 30


def _header(title: str) -> None:
    print()
    print(bold("─" * 80))
    print(bold(f"  {title}"))
    print(bold("─" * 80))


def _section(title: str) -> None:
    print()
    print(yellow(f"  ── {title}"))


def _row(label: str, result: str, note: str = "") -> None:
    note_str = dim(f"   {note}") if note else ""
    print(f"  {label:<{_W_LABEL}}{cyan(f'{result:<{_W_RESULT}}')}{note_str}")


def _skip(label: str, reason: str) -> None:
    print(f"  {label:<{_W_LABEL}}{red('SKIPPED')}  {dim(reason)}")


def _comparison_row(label: str, val_ms: float, baseline_ms: float | None) -> None:
    """Print a row with an optional speedup/slowdown indicator vs *baseline_ms*."""
    val_str = f"{val_ms:.2f} ms"
    if baseline_ms is not None and baseline_ms > 0:
        ratio = baseline_ms / val_ms
        if ratio >= 1.0:
            indicator = green(f"  {ratio:.1f}× faster")
        else:
            indicator = dim(f"  {1/ratio:.1f}× slower")
    else:
        indicator = ""
    print(f"  {label:<{_W_LABEL}}{cyan(f'{val_str:<{_W_RESULT}}')}{indicator}")

# ---------------------------------------------------------------------------
# Benchmark groups
# ---------------------------------------------------------------------------


def bench_construction(n: int = 200) -> float:
    """Time pure ParamDB() construction (lazy — no I/O)."""
    times = _bench(lambda: ParamDB(), n=n)
    _row("ParamDB() construction", _fmt(times), f"n={n}, lazy — no file read")
    return _mean_ms(times)


def bench_cold_offline(n: int = 5) -> float:
    """Cold-load: first lookup from the bundled YAML."""
    def _run():
        db = ParamDB()
        db.param_id_to_shortname(130)   # triggers YAML load

    times = _bench(_run, n=n)
    _row("Offline — cold load (bundled YAML)", _fmt(times), f"n={n}")
    return _mean_ms(times)


def bench_cold_online_miss(n: int = 3, cache_path: Path | None = None) -> float | None:
    """Cold-load: first lookup with a guaranteed cache miss (hits the API)."""
    from datetime import timedelta

    results: list[float] = []
    for _ in range(n):
        # Each run gets its own empty temp dir → guaranteed cache miss.
        with tempfile.TemporaryDirectory() as tmp:
            try:
                gc.collect()
                t0 = time.perf_counter_ns()
                db = ParamDB(
                    mode="online",
                    cache_path=tmp,
                    cache_ttl=timedelta(hours=1),
                )
                db.param_id_to_shortname(130)
                results.append((time.perf_counter_ns() - t0) / _NS_PER_MS)
            except Exception as exc:
                _skip("Online — cold load (API, cache miss)", str(exc))
                return None

    _row("Online — cold load (API, cache miss)", _fmt(results), f"n={n}")
    return _mean_ms(results)


def bench_cold_online_hit(n: int = 5, cache_path: Path | None = None) -> float | None:
    """Cold-load: first lookup with a warm JSON cache."""
    from datetime import timedelta

    effective_cache = cache_path or Path(tempfile.mkdtemp(prefix="paramdb_bench_"))

    # Pre-warm the cache (one network call).
    try:
        db_prime = ParamDB(mode="online", cache_path=effective_cache)
        db_prime.param_id_to_shortname(130)
    except Exception as exc:
        _skip("Online — cold load (JSON cache hit)", f"cache warm-up failed: {exc}")
        return None

    def _run():
        db = ParamDB(
            mode="online",
            cache_path=effective_cache,
            cache_ttl=timedelta(hours=1),
        )
        db.param_id_to_shortname(130)

    try:
        times = _bench(_run, n=n)
    except Exception as exc:
        _skip("Online — cold load (JSON cache hit)", str(exc))
        return None

    _row("Online — cold load (JSON cache hit)", _fmt(times), f"n={n}")
    return _mean_ms(times)


def bench_warm_lookups(db: ParamDB, n: int = 500) -> None:
    """Benchmark every public lookup method on an already-loaded ParamDB."""
    cases: list[tuple[str, object]] = [
        # ── ID-based ──────────────────────────────────────────────────────
        ("param_id_to_shortname(130)",
            lambda: db.param_id_to_shortname(130)),
        ("param_id_to_longname(130)",
            lambda: db.param_id_to_longname(130)),
        # ── Shortname → ID / longname (no collision) ──────────────────────
        ("shortname_to_param_id('strf')  [unique]",
            lambda: db.shortname_to_param_id("strf")),
        ("shortname_to_longname('strf')  [unique]",
            lambda: db.shortname_to_longname("strf")),
        # ── Shortname → ID (collision, default resolution) ────────────────
        ("shortname_to_param_id('t')     [collision, default]",
            lambda: db.shortname_to_param_id("t")),
        ("shortname_to_param_id('tp')    [collision, default]",
            lambda: db.shortname_to_param_id("tp")),
        # ── Shortname → ID (collision, explicit context) ──────────────────
        ("shortname_to_param_id('t', table=128)",
            lambda: db.shortname_to_param_id("t", table=128)),
        ("shortname_to_param_id('t', origin=98)",
            lambda: db.shortname_to_param_id("t", origin=98)),
        ("shortname_to_param_id('t', access='dissemination')",
            lambda: db.shortname_to_param_id("t", access="dissemination")),
        # ── Longname-based ────────────────────────────────────────────────
        ("longname_to_param_id('Temperature')",
            lambda: db.longname_to_param_id("Temperature")),
        ("longname_to_shortname('Temperature')",
            lambda: db.longname_to_shortname("Temperature")),
        # ── get_metadata / get_units ──────────────────────────────────────
        ("get_metadata(130)              [by id]",
            lambda: db.get_metadata(130)),
        ("get_metadata('t')              [by shortname]",
            lambda: db.get_metadata("t")),
        ("get_metadata('Temperature')    [by longname]",
            lambda: db.get_metadata("Temperature")),
        ("get_units(130)",
            lambda: db.get_units(130)),
        # ── Collision inspection ──────────────────────────────────────────
        ("get_all_by_shortname('strf')   [unique, 1 result]",
            lambda: db.get_all_by_shortname("strf")),
        ("get_all_by_shortname('t')      [collision, 2 results]",
            lambda: db.get_all_by_shortname("t")),
        ("shortname_has_collisions('strf')",
            lambda: db.shortname_has_collisions("strf")),
        ("shortname_has_collisions('t')",
            lambda: db.shortname_has_collisions("t")),
    ]

    for label, fn in cases:
        times = _bench(fn, n=n)
        _row(label, _fmt(times))

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main() -> None:
    global _USE_COLOR

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--online", action="store_true",
        help="Include online mode benchmarks (requires network on first run)",
    )
    parser.add_argument(
        "--cache-path", default=None,
        help="Cache directory for online mode (default: OS user-cache dir)",
    )
    parser.add_argument(
        "--iterations", type=int, default=500,
        help="Number of iterations for warm lookups (default: 500)",
    )
    parser.add_argument(
        "--no-color", action="store_true",
        help="Disable ANSI colour output",
    )
    args = parser.parse_args()

    if args.no_color or not sys.stdout.isatty():
        _USE_COLOR = False

    n_warm      = args.iterations
    n_cold      = 5
    cache_path  = Path(args.cache_path) if args.cache_path else None

    print(bold("\nParamDB benchmark"))
    print(dim(f"  Python {sys.version.split()[0]}  |  pymetkit.ParamDB  |  {n_warm} warm-lookup iterations"))

    # ── Construction (lazy — same for both modes) ──────────────────────────
    _header("Construction  (lazy — no I/O at init)")
    _section("Both modes")
    bench_construction(n=200)

    # ── Offline ────────────────────────────────────────────────────────────
    _header("Offline mode  (bundled YAML)")

    _section(f"Cold load  (n={n_cold})")
    print(dim("  (first lookup triggers YAML parse + Pydantic validation of all ~8,200 entries)"))
    t_offline_cold = bench_cold_offline(n=n_cold)

    _section(f"Warm lookups  (n={n_warm} each)")
    db_offline = ParamDB()
    db_offline.param_id_to_shortname(1)   # ensure loaded before timing starts
    bench_warm_lookups(db_offline, n=n_warm)

    # ── Online ─────────────────────────────────────────────────────────────
    t_online_miss: float | None = None
    t_online_hit:  float | None = None

    if args.online:
        _header("Online mode  (ECMWF parameter database API)")

        _section(f"Cold load: cache miss — live API fetch  (n={n_cold})")
        print(dim("  (each run uses a fresh empty cache dir to force a network request)"))
        print(dim("  (result includes HTTP round-trip + JSON parse + Pydantic validation)"))
        t_online_miss = bench_cold_online_miss(n=n_cold)

        _section(f"Cold load: cache hit — local JSON  (n={n_cold})")
        print(dim("  (JSON parse + Pydantic validation; no network)"))
        t_online_hit = bench_cold_online_hit(n=n_cold, cache_path=cache_path)

        _section(f"Warm lookups  (n={n_warm} each)")
        print(dim("  (in-memory lookups are identical to offline — shown for completeness)"))
        if t_online_hit is not None:
            from datetime import timedelta
            _cp = cache_path or Path(tempfile.mkdtemp(prefix="paramdb_bench_"))
            db_online = ParamDB(mode="online", cache_path=_cp,
                                cache_ttl=timedelta(hours=1))
            try:
                db_online.param_id_to_shortname(1)
                bench_warm_lookups(db_online, n=n_warm)
            except Exception as exc:
                print(dim(f"  Skipped: {exc}"))
        else:
            print(dim("  Skipped — online load failed."))

    # ── Comparison summary ─────────────────────────────────────────────────
    if args.online and (t_online_miss is not None or t_online_hit is not None):
        _header("Cold-load comparison  (mean time per fresh ParamDB + first lookup)")
        _comparison_row(
            "Offline   — bundled YAML",
            t_offline_cold,
            baseline_ms=None,
        )
        if t_online_hit is not None:
            _comparison_row(
                "Online    — JSON cache hit",
                t_online_hit,
                baseline_ms=t_offline_cold,
            )
        if t_online_miss is not None:
            _comparison_row(
                "Online    — live API (cache miss)",
                t_online_miss,
                baseline_ms=t_offline_cold,
            )
        print()
        print(dim("  Note: warm lookups are O(1) dict access and identical across modes"))
        print(dim("  after the initial load — the cold-load time is the only meaningful"))
        print(dim("  difference between offline and online."))

    print()
    print(bold("Done."))
    print()


if __name__ == "__main__":
    main()
