[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/ecmwf/metkit/blob/develop/LICENSE)

# metkit Rust bindings

Rust bindings for [metkit](https://github.com/ecmwf/metkit), ECMWF's toolkit
for processing meteorological requests.

- **`metkit`** — safe Rust wrapper: `MarsRequest` building/expansion,
  `CodesHandle` GRIB access, `HyperCube`, protocol environment metadata.
- **`metkit-sys`** — raw C++ bindings via [cxx](https://cxx.rs), including an
  auto-generated typed `Error` enum derived from metkit's exception headers.

## Requirements

Build dependencies:

- CMake --- For use and installation see http://www.cmake.org/
- C++17 compiler

Runtime dependencies (built from source or found on the system, depending on
the build mode):

- eckit -- http://github.com/ecmwf/eckit
- eccodes -- http://github.com/ecmwf/eccodes (for the `grib` / `bufr` features)

## Build modes

Exactly one build strategy feature must be enabled:

- `vendored` (default) --- clone and build the pinned metkit version from
  source with [ecbuild](https://github.com/ecmwf/ecbuild) (fetched
  automatically).
- `system` --- link against an installed metkit found via CMake
  `find_package`; set `METKIT_DIR` or `CMAKE_PREFIX_PATH` to locate it.

## Features

| Feature | Default | Description |
|---|---|---|
| `grib` | yes | GRIB format support (via eccodes) |
| `bufr` | yes | BUFR format support (via eccodes) |
| `mars2grib` | yes | MARS2GRIB encoder |
| `metkit-config` | yes | Install metkit configuration files |
| `netcdf` | no | NetCDF data support |
| `odb` | no | ODB data support |
| `experimental` | no | Experimental features |

## License

Copyright 2021 European Centre for Medium-Range Weather Forecasts (ECMWF)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
> http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

In applying this licence, ECMWF does not waive the privileges and immunities granted to it by virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
