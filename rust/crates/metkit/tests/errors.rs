// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

//! C++ exceptions reach Rust as the matching `Error` variant.

use metkit::{CodesHandle, Error, Expanded, MarsRequest};

#[test]
fn expansion_failure_is_user_error() {
    eckit::init();
    let Err(err) = "retrieve, class=od, foo=bar".parse::<MarsRequest<Expanded>>() else {
        panic!("unknown keyword expanded");
    };
    assert!(
        matches!(err, Error::Eckit(eckit::Error::UserError(_))),
        "got {err:?}"
    );
}

#[test]
fn missing_grib_key_is_codes_exception() {
    eckit::init();
    let handle = CodesHandle::from_sample("GRIB2").expect("GRIB2 sample");
    let err = handle
        .get::<i64>("noSuchKey")
        .expect_err("unknown key read");
    assert!(
        matches!(err, Error::Metkit(metkit_sys::Error::CodesException(_))),
        "got {err:?}"
    );
}
