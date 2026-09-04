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

## Command line

```bash
python -m pymetkit --print-home        # metkit library home
python -m pymetkit --print-home-deps   # all dependency homes and versions
```
