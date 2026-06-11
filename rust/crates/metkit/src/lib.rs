//! Safe Rust wrapper for ECMWF's metkit C++ library.
//!
//! Provides:
//! - [`MarsRequestBuilder`] / [`MarsRequest`] — build and validate MARS requests
//! - [`MarsLanguage`] — keyword categorization
//! - [`initialize_environment`] — protocol environment metadata

pub mod codes;
pub mod environment;
pub mod error;
pub mod handle;
pub mod hypercube;
pub mod language;
pub mod param;
pub mod request;

pub use codes::{CodesGet, CodesHandle, CodesSet};
pub use environment::initialize;
pub use error::{Error, Result};
pub use handle::MarsRequestHandle;
pub use hypercube::HyperCube;
pub use language::MarsLanguage;
pub use param::{WindFamily, wind_families};
pub use request::{MarsRequest, MarsRequestBuilder, ParsedRequests, parse, tokenize};
