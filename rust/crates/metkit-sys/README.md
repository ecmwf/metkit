# metkit-sys

Low-level Rust bindings to ECMWF's [metkit](https://github.com/ecmwf/metkit) C++
library.

This crate provides raw FFI bindings using [cxx](https://cxx.rs/). For a safe,
ergonomic API, use the higher-level `metkit` crate (forthcoming).

## Cargo build features

These flags control what the underlying C++ metkit library is compiled with.
Each maps to a `-DENABLE_<NAME>=ON/OFF` flag in upstream
[`metkit/CMakeLists.txt`](https://github.com/ecmwf/metkit/blob/develop/CMakeLists.txt).

### Build strategy (mutually exclusive)

- `vendored` (default) - Clone and build metkit (and its eckit and ecCodes
  dependencies) from source.
- `system` - Link against a system-installed metkit, located via CMake
  `find_package(metkit)`. Honours `METKIT_DIR` and `CMAKE_PREFIX_PATH`.

### Format support (enabled by default)

- `grib` - GRIB format support. Pulls in `eccodes-sys/product-grib`.
- `bufr` - BUFR format support. Pulls in `eccodes-sys/product-bufr`.

### Format support (off by default; require external libraries)

- `netcdf` - NetCDF data support.
- `odb` - ODB data support (requires `odc`).

### Encoding (enabled by default)

- `mars2grib` - MARS2GRIB encoder. Pulls in `eckit-sys/geo-codec-grids` for
  ORCA / FESOM / ICON grid support.

### Configuration (enabled by default)

- `metkit-config` - Install metkit configuration files (e.g. `language.yaml`).

### Other (off by default)

- `experimental` - Experimental upstream features.
- `fail-on-ccsds` - Fail when encountering CCSDS-encoded messages.

## Environment variables

- `METKIT_DIR` - Install prefix of a metkit build, used by `system` mode.
- `CMAKE_PREFIX_PATH` - Additional CMake search paths.
- `DOCS_RS` - When set, the build script becomes a no-op (for docs.rs).

## License

Apache-2.0
