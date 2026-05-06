# metkit-sys

Low-level Rust bindings to ECMWF's [metkit](https://github.com/ecmwf/metkit) C++ library.

This crate provides raw FFI bindings using [cxx](https://cxx.rs/). For a safe, ergonomic API, use the higher-level `metkit` crate (planned).

## Features

### Build strategy (mutually exclusive)

- `vendored` - Build metkit and its dependencies (eckit, ecCodes) from source.
- `system` - Link against system-installed metkit.

`vendored` is enabled by default.

### Format support (enabled by default)

- `grib` - GRIB format support. Pulls in `eccodes-sys/product-grib`.
- `bufr` - BUFR format support. Pulls in `eccodes-sys/product-bufr`.

### Format support (off by default; require external libraries)

- `netcdf` - NetCDF data support (requires NetCDF library).
- `odb` - ODB data support (requires odc).

### Encoding (enabled by default)

- `mars2grib` - MARS2GRIB encoder. Pulls in `eckit-sys/geo-codec-grids` for ORCA/FESOM/ICON grid support.

### Configuration (enabled by default)

- `metkit-config` - Install metkit configuration files (e.g. `language.yaml`).

### Other (off by default)

- `experimental` - Experimental upstream features.
- `fail-on-ccsds` - Fail when encountering CCSDS-encoded messages.

## License

Apache-2.0
