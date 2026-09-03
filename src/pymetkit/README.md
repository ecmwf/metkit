# pymetkit

[![Static Badge](https://github.com/ecmwf/codex/raw/refs/heads/main/Project%20Maturity/emerging_badge.svg)](https://github.com/ecmwf/codex/raw/refs/heads/main/Project%20Maturity#emerging)

> \[!IMPORTANT\]
> This software is **Emerging** and subject to ECMWF's guidelines on [Software Maturity](https://github.com/ecmwf/codex/raw/refs/heads/main/Project%20Maturity).

`pymetkit` is a Python interface to [metkit](https://github.com/ecmwf/metkit), ECMWF's
meteorological toolkit. It exposes the MARS request model in a Pythonic way. 

The native `libmetkit` shared library and its dependencies are located at runtime via
[findlibs](https://github.com/ecmwf/findlibs).

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

## Documentation

For implementation details and tooling, see the [Metkit project pages](https://sites.ecmwf.int/docs/metkit).

To build the latest documentation locally, follow the guide at [Metkit](https://github.com/ecmwf/metkit).


## License

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/ecmwf/metkit/blob/develop/LICENSE)
