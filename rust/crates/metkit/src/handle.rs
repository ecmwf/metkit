//! `MarsRequestHandle` — `DataHandle` for Hermes protocol operations.
//!
//! Mirrors C++ `metkit::mars::MarsRequestHandle`.
//! Wraps the Hermes TCP protocol: connects on `open_for_read()`,
//! reads GRIB data via `Read` trait.

use crate::request::{Expanded, MarsRequest};

/// A `DataHandle` that retrieves data via the Hermes protocol.
///
/// Created from a `MarsRequest` and a database configuration.
/// The connection and protocol handshake happen on `open_for_read()`.
///
/// # Example
///
/// ```ignore
/// let handle = MarsRequestHandle::new(&request, &db_config)?;
/// let (handle, len) = handle.open_for_read()?;
/// for msg in eckit::MessageReader::new(&mut handle)? {
///     // ...
/// }
/// ```
pub struct MarsRequestHandle {
    inner: eckit::DataHandle,
}

impl MarsRequestHandle {
    /// Create a new handle from an expanded request and database configuration.
    ///
    /// Mirrors C++ `MarsRequestHandle(request, config)`. The request is sent
    /// to the server as-is, so it must already be language-expanded.
    pub fn new(request: &MarsRequest<Expanded>, config: &eckit::Config) -> crate::Result<Self> {
        let inner = request
            .as_sys()
            .make_handle(config.as_sys())
            .map_err(crate::Error::from)?;
        Ok(Self {
            inner: eckit::DataHandle::from_raw(inner),
        })
    }

    /// Open for reading — connects to the server and performs the Hermes handshake.
    ///
    /// Returns the opened handle and the estimated data length.
    pub fn open_for_read(self) -> crate::Result<(eckit::DataHandle<eckit::Reading>, i64)> {
        self.inner.open_for_read().map_err(crate::Error::from)
    }
}

impl From<MarsRequestHandle> for eckit::DataHandle<eckit::Closed> {
    fn from(handle: MarsRequestHandle) -> Self {
        handle.inner
    }
}
