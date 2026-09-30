<!--
SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
SPDX-License-Identifier: Apache-2.0
-->

# metkit

Safe Rust wrapper for ECMWF's [metkit](https://github.com/ecmwf/metkit) C++ library.

## Overview

This crate provides a safe API on top of the low-level
[`metkit-sys`](https://crates.io/crates/metkit-sys) bindings:

- `MarsRequest` - build, parse and expand MARS requests, with typestate
  `Raw` / `Expanded` tracking whether the MARS language expansion has run
- `CodesHandle` - typed get/set access to GRIB message keys
- `HyperCube` - request hypercube arithmetic
- `initialize` / `environment_request` - protocol request environment metadata
- `Error` - metkit C++ exceptions surfaced as typed Rust errors

## Installation

Add the crate to your `Cargo.toml`:

```toml
[dependencies]
metkit = "1.21"
```

The default `vendored` feature builds the metkit C++ library (and its eckit
and ecCodes dependencies) from source, which requires CMake and a C++17
compiler.

## Usage

```rust,no_run
use metkit::{Expanded, MarsRequest};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Build a raw request, then expand it against the MARS language
    let mut request = MarsRequest::new("retrieve");
    request.set("class", "od");
    request.set("date", "-1");
    let expanded = request.expand(true, false)?;
    println!("{}", expanded.dump());

    // Or parse MARS text directly (parsing runs the expansion)
    let parsed: MarsRequest<Expanded> = "retrieve, class=od, date=-1".parse()?;
    println!("{}", parsed.dump());
    Ok(())
}
```

## Cargo build features

- `vendored` (default) - Build the metkit C++ library from source
  (forwards `metkit-sys/vendored`).
- `system` - Link against a system-installed metkit
  (forwards `metkit-sys/system`).
- `grib` (default) - GRIB format support (forwards `metkit-sys/grib`).
- `bufr` (default) - BUFR format support (forwards `metkit-sys/bufr`).

See the [`metkit-sys` README](https://crates.io/crates/metkit-sys) for the
full set of underlying C++ build features and environment variables.

## Copyright and License

Copyright 1996- European Centre for Medium-Range Weather Forecasts (ECMWF).

This software is licensed under the terms of the [Apache License, Version 2.0](LICENSE) which can also be obtained at http://www.apache.org/licenses/LICENSE-2.0.

In applying this licence, ECMWF does not waive the privileges and immunities granted to it by virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
