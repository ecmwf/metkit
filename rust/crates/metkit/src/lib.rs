//! Safe Rust wrapper for ECMWF's metkit C++ library.
//!
//! Provides:
//! - [`MarsRequest`] — build and validate MARS requests (typestate: [`Raw`] / [`Expanded`])
//! - [`initialize`] — protocol environment metadata

pub mod codes;
pub mod environment;
pub mod error;
pub mod handle;
pub mod hypercube;
pub mod request;

pub use codes::{CodesGet, CodesHandle, CodesSet};
pub use environment::initialize;
pub use error::{Error, Result};
pub use handle::MarsRequestHandle;
pub use hypercube::HyperCube;
pub use request::{
    Expanded, MarsRequest, MarsValue, ParsedRequests, Raw, RequestState, parse, tokenize,
};
