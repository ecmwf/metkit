//! Error types for metkit operations.
//!
//! Wraps both the auto-generated [`metkit_sys::Error`] (metkit's own
//! `metkit::codes::CodesException` family) and the [`eckit_sys::Error`]
//! exceptions that surface through the same bridge. A bare `cxx::Exception`
//! whose message matches neither namespace is preserved as `Other`.
//!
//! # Example
//!
//! Pattern-match on the typed variants to recover from specific failures
//! (and route everything else to a fallback):
//!
//! ```no_run
//! use metkit::{CodesHandle, Error, Result};
//! use metkit_sys::Error as MetkitError;
//! use eckit::Error as EckitError;
//!
//! fn process_grib(path: &str, key: &str) -> Result<String> {
//!     let h = CodesHandle::from_file(path)?;   // may throw eckit::FileError
//!     let value: String = h.get(key)?;          // may throw metkit::CodesException
//!     Ok(value)
//! }
//!
//! match process_grib("/tmp/foo.grib", "shortName") {
//!     Ok(v) => println!("shortName = {v}"),
//!
//!     // Typed metkit error — recover or report with metkit-specific context
//!     Err(Error::Metkit(MetkitError::CodesException(msg))) => {
//!         eprintln!("ecCodes rejected the key: {msg}");
//!     }
//!     Err(Error::Metkit(MetkitError::CodesWrongLength(msg))) => {
//!         eprintln!("buffer length mismatch: {msg}");
//!     }
//!
//!     // Typed eckit error — fall back, retry, or wrap
//!     Err(Error::Eckit(EckitError::FileError(msg))) => {
//!         eprintln!("file error: {msg} (will retry with default sample)");
//!     }
//!     Err(Error::Eckit(EckitError::UserError(msg))) => {
//!         eprintln!("bad input: {msg}");
//!     }
//!
//!     // Untyped fallback
//!     Err(other) => eprintln!("unhandled: {other}"),
//! }
//! ```

use eckit::Error as EckitError; // re-export of eckit_sys::Error
use metkit_sys::Error as MetkitError;

/// Combined error type for metkit operations.
#[derive(Debug, thiserror::Error)]
pub enum Error {
    /// Typed metkit exception (e.g. `CodesException`, `CodesWrongLength`).
    #[error(transparent)]
    Metkit(#[from] MetkitError),
    /// Typed eckit exception (e.g. `UserError`, `FileError`, `BadParameter`).
    #[error(transparent)]
    Eckit(#[from] EckitError),
    /// Exception from neither namespace; raw message preserved.
    #[error("{0}")]
    Other(String),
}

impl From<cxx::Exception> for Error {
    fn from(e: cxx::Exception) -> Self {
        if let Some(metkit) = MetkitError::try_from_cxx(&e) {
            return Self::Metkit(metkit);
        }
        if let Some(eckit) = EckitError::try_from_cxx(&e) {
            return Self::Eckit(eckit);
        }
        Self::Other(e.what().to_string())
    }
}

pub type Result<T> = std::result::Result<T, Error>;
