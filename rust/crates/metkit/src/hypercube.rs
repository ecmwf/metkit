//! `HyperCube` — N-dimensional field indexing and counting.
//!
//! Wraps `metkit::hypercube::HyperCube`. Used for:
//! - Counting expected fields in a request (`size()`)
//! - Tracking which fields have been retrieved (`clear()`, `count()`)
//! - Ordering fields in a fieldset (`field_ordinal()`)

use crate::request::{Expanded, MarsRequest, RequestState};

/// N-dimensional field grid derived from a `MarsRequest`.
pub struct HyperCube {
    inner: metkit_sys::UniquePtr<metkit_sys::HyperCubeWrapper>,
}

// SAFETY: HyperCube is accessed through &self / &mut self only.
#[allow(clippy::non_send_fields_in_send_ty)]
unsafe impl Send for HyperCube {}
unsafe impl Sync for HyperCube {}

impl HyperCube {
    /// Create from an expanded `MarsRequest` — builds the N-dimensional grid
    /// from the request's parameter values.
    ///
    /// The C++ `HyperCube` assumes canonical, enumerated values (no date
    /// ranges, no aliases), so only [`MarsRequest<Expanded>`] is accepted.
    pub fn new(request: &MarsRequest<Expanded>) -> crate::Result<Self> {
        let inner =
            metkit_sys::HyperCubeWrapper::create(request.as_sys()).map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Total number of cells in the hypercube.
    #[must_use]
    pub fn size(&self) -> usize {
        self.inner.size()
    }

    /// Number of remaining (uncleared) cells.
    #[must_use]
    pub fn count(&self) -> usize {
        self.inner.count()
    }

    /// Number of vacant (uncleared) cells.
    #[must_use]
    pub fn count_vacant(&self) -> usize {
        self.inner.count_vacant()
    }

    /// Check if a request matches a cell in the hypercube.
    ///
    /// Field requests often come from GRIB messages
    /// (`TryFrom<&eckit::Message>`, which yields `MarsRequest<Raw>`),
    /// so any state is accepted.
    pub fn contains<S: RequestState>(&self, request: &MarsRequest<S>) -> crate::Result<bool> {
        self.inner
            .contains(request.as_sys())
            .map_err(crate::Error::from)
    }

    /// Mark a cell as found. Returns true if the cell existed and was cleared.
    pub fn clear<S: RequestState>(&mut self, request: &MarsRequest<S>) -> crate::Result<bool> {
        self.inner
            .pin_mut()
            .clear(request.as_sys())
            .map_err(crate::Error::from)
    }

    /// Get the ordinal position of a request in the hypercube (for field ordering).
    pub fn field_ordinal<S: RequestState>(&self, request: &MarsRequest<S>) -> crate::Result<usize> {
        self.inner
            .field_ordinal(request.as_sys())
            .map_err(crate::Error::from)
    }
}
