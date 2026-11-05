//! Error types for metkit operations.
//!
//! Metkit's own exception classes are variants of [`Error`]; eckit
//! exceptions that surface through the same bridge are nested as
//! [`Error::Eckit`]. A `cxx::Exception` whose message matches neither
//! namespace is preserved as [`Error::Other`].
//!
//! # Example
//!
//! ```no_run
//! use metkit::{CodesHandle, Error, Result};
//!
//! fn process_grib(path: &str, key: &str) -> Result<String> {
//!     let h = CodesHandle::from_file(path)?;
//!     let value: String = h.get(key)?;
//!     Ok(value)
//! }
//!
//! match process_grib("/tmp/foo.grib", "shortName") {
//!     Ok(v) => println!("shortName = {v}"),
//!     Err(Error::CodesException(msg)) => eprintln!("ecCodes rejected the key: {msg}"),
//!     Err(Error::Eckit(eckit::Error::CantOpenFile(msg))) => eprintln!("cannot open: {msg}"),
//!     Err(other) => eprintln!("{other}"),
//! }
//! ```

/// Error type for metkit operations.
#[derive(Debug, thiserror::Error)]
pub enum Error {
    /// `metkit::CodesException`
    #[error("metkit::CodesException: {0}")]
    CodesException(String),
    /// `metkit::CodesWrongLength`
    #[error("metkit::CodesWrongLength: {0}")]
    CodesWrongLength(String),
    /// Typed eckit exception (e.g. `UserError`, `FileError`, `BadParameter`).
    #[error(transparent)]
    Eckit(#[from] eckit::Error),
    /// Exception from neither namespace; raw message preserved.
    #[error("{0}")]
    Other(String),
}

impl From<metkit_sys::Error> for Error {
    fn from(e: metkit_sys::Error) -> Self {
        match e {
            metkit_sys::Error::CodesException(msg) => Self::CodesException(msg),
            metkit_sys::Error::CodesWrongLength(msg) => Self::CodesWrongLength(msg),
            metkit_sys::Error::Other(msg) => Self::Other(msg),
        }
    }
}

impl From<cxx::Exception> for Error {
    fn from(e: cxx::Exception) -> Self {
        if let Some(metkit) = metkit_sys::Error::try_from_cxx(&e) {
            return Self::from(metkit);
        }
        if let Some(eckit) = eckit::Error::try_from_cxx(&e) {
            return Self::Eckit(eckit);
        }
        Self::Other(e.what().to_string())
    }
}

pub type Result<T> = std::result::Result<T, Error>;
