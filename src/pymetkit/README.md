# pymetkit

`pymetkit` is a Python interface to [metkit](https://github.com/ecmwf/metkit), ECMWF's
meteorological toolkit. It exposes the MARS request model in a Pythonic way, built on a
[pybind11](https://github.com/pybind/pybind11) extension module (`pymetkit_bindings`) that binds the
metkit C++ library directly.

The native `libmetkit` shared library and its dependencies are located at runtime via
[findlibs](https://github.com/ecmwf/findlibs).

## Architecture

- `pymetkit_bindings` — compiled pybind11 module binding `metkit::mars::MarsRequest` and
  `metkit::mars::MarsExpansion`.
- `pymetkit._internal` — loads the native library via `findlibs`, initialises the bindings, and
  re-exports the raw symbols.
- `pymetkit` — the Pythonic layer: `MarsRequest` (a verb plus a `MarsSelection`),
  `MarsSelection` (a type alias for the user-facing key-value mapping),
  `UserInputMapper` (normalises `MarsSelection` values to and from the internal
  `dict[str, list[str]]` representation), and `parse_mars_request`.

## Usage

```python
from pymetkit import MarsRequest, parse_mars_request

# Build a request from a verb and a selection
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

# Expand against the MARS language definition
expanded = request.expand()
print(expanded.verb(), dict(expanded))

# Parse requests from a string or a file
requests = parse_mars_request("retrieve,class=od,date=-1,param=129,step=12")
```

## ParamDB — parameter database

`ParamDB` maps between ECMWF short names, long names and numeric parameter IDs, backed by a
bundled `parameter_metadata.json` (with a `parameter_metadata.yaml` fallback) or, in
`mode="online"`, the ECMWF parameter API.

```python
from pymetkit import ParamDB, AmbiguousParamError

db = ParamDB()                       # mode="offline" by default; data loads lazily

db.shortname_to_param_id("msl")      # 151  — unambiguous
db.param_id_to_shortname(151)        # "msl"
db.shortname_to_longname("2t")       # "2 metre temperature"
db.get_units(167)                    # "K"
```

### Ambiguous short names

Some short names map to more than one parameter ID (e.g. `tp` → `228` and `228228`). ParamDB
never guesses — an ambiguous lookup **raises** by default:

```python
try:
    db.shortname_to_param_id("tp")
except AmbiguousParamError as exc:
    print(exc.shortname)             # "tp"
    for cand in exc.candidates:      # every ParamIDCandidate, sorted
        print(cand.param_id, cand.table)
```

You can resolve the ambiguity in three ways:

```python
# 1. Narrow with a MARS context (resolved via the C++ expand engine)
db.shortname_to_param_id("tp", context={"class": "od"})   # 228

# 2. Narrow with hard metadata filters (no MARS request constructed)
db.shortname_to_param_id("tp", table=128)                 # 228

# 3. Accept the canonical (first-sorted, lowest-table/id) candidate
db.shortname_to_param_id("tp", default=True)              # 228
```

To inspect the options programmatically instead of catching the error, use
`shortname_to_param_id_candidates`, which returns a list of `ParamIDCandidate`
(`param_id`, `table`, `origin`, `access`, `mars_request_context`):

```python
for cand in db.shortname_to_param_id_candidates("tp"):
    print(cand.param_id, cand.hard_filter_selector)
```

> **Note:** Per-candidate MARS context computation is temporarily deferred, so every returned
> or raised `ParamIDCandidate` currently carries `mars_request_context=None`. Passing
> `context=` to *narrow* a lookup still works; only the *advertised* selecting context is
> unavailable for now. The `context=` path additionally requires the compiled
> `pymetkit._internal` extension — without it, ParamDB falls back to the baked metadata.

## Command line

```bash
python -m pymetkit --print-home        # metkit library home
python -m pymetkit --print-home-deps   # all dependency homes and versions
```
