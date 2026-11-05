//! `CodesHandle` — GRIB/BUFR field access via eccodes.
//!
//! Wraps `metkit::codes::CodesHandle` for reading and modifying
//! individual GRIB/BUFR messages.

use std::path::Path;

/// Handle to a GRIB/BUFR message via eccodes.
pub struct CodesHandle {
    inner: metkit_sys::UniquePtr<metkit_sys::CodesHandleWrapper>,
}

// SAFETY: CodesHandle owns a UniquePtr to a C++ object that can be moved between threads.
// NOT Sync: eccodes handles have internal mutable state (accessor caches, key iterators)
// even on const methods — concurrent reads from multiple threads would be a data race.
// Use Arc<Mutex<CodesHandle>> if shared access is needed.
#[allow(clippy::non_send_fields_in_send_ty)]
unsafe impl Send for CodesHandle {}

impl CodesHandle {
    /// Create from raw message bytes (copies the data).
    pub fn from_message(data: &[u8]) -> crate::Result<Self> {
        let inner =
            metkit_sys::CodesHandleWrapper::from_message(data).map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Create from an eccodes sample (e.g. `"GRIB1"`, `"GRIB2"`).
    ///
    /// Mirrors C++ `metkit::codes::codesHandleFromSample()`.
    pub fn from_sample(sample: impl AsRef<str>) -> crate::Result<Self> {
        let inner = metkit_sys::CodesHandleWrapper::from_sample(sample.as_ref())
            .map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Create from a GRIB file.
    pub fn from_file(path: impl AsRef<Path>) -> crate::Result<Self> {
        let path_str = path.as_ref().to_str().ok_or_else(|| {
            crate::Error::Other(format!(
                "path is not valid UTF-8: {}",
                path.as_ref().display()
            ))
        })?;
        let inner =
            metkit_sys::CodesHandleWrapper::from_file(path_str).map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Create from a GRIB file at a specific byte offset.
    pub fn from_file_at_offset(path: impl AsRef<Path>, offset: i64) -> crate::Result<Self> {
        let path_str = path.as_ref().to_str().ok_or_else(|| {
            crate::Error::Other(format!(
                "path is not valid UTF-8: {}",
                path.as_ref().display()
            ))
        })?;
        let inner = metkit_sys::CodesHandleWrapper::from_file_at_offset(path_str, offset)
            .map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Check if a key is defined.
    pub fn is_defined(&self, key: &str) -> crate::Result<bool> {
        self.inner.is_defined(key).map_err(crate::Error::from)
    }

    /// Check if a key is missing.
    pub fn is_missing(&self, key: &str) -> crate::Result<bool> {
        self.inner.is_missing(key).map_err(crate::Error::from)
    }

    /// Check if a key exists and has a value.
    pub fn has(&self, key: &str) -> crate::Result<bool> {
        self.inner.has(key).map_err(crate::Error::from)
    }

    /// Get a typed value.
    pub fn get<T: CodesGet>(&self, key: &str) -> crate::Result<T> {
        T::get_from(&self.inner, key)
    }

    /// Set a typed value.
    pub fn set<T: CodesSet>(&mut self, key: &str, value: T) -> crate::Result<()> {
        value.set_on(self.inner.pin_mut(), key)
    }

    /// Set a key to its missing value.
    pub fn set_missing(&mut self, key: &str) -> crate::Result<()> {
        self.inner
            .pin_mut()
            .set_missing(key)
            .map_err(crate::Error::from)
    }

    /// Number of elements for a key (1 for scalars, N for arrays).
    pub fn value_count(&self, key: &str) -> crate::Result<usize> {
        self.inner.value_count(key).map_err(crate::Error::from)
    }

    /// Size of the raw message in bytes.
    pub fn message_size(&self) -> crate::Result<usize> {
        self.inner.message_size().map_err(crate::Error::from)
    }

    /// Raw message data (no copy — lifetime bound to this handle).
    pub fn message_data(&self) -> crate::Result<&[u8]> {
        self.inner.message_data().map_err(crate::Error::from)
    }

    /// Access the underlying metkit-sys wrapper.
    #[must_use]
    pub fn as_sys(&self) -> &metkit_sys::CodesHandleWrapper {
        &self.inner
    }
}

impl CodesHandle {
    /// Clone the underlying eccodes handle.
    pub fn try_clone(&self) -> crate::Result<Self> {
        let inner = self.inner.clone_handle().map_err(crate::Error::from)?;
        Ok(Self { inner })
    }
}

// ==================== Generic get/set traits ====================

/// Trait for types that can be read from a `CodesHandle`.
pub trait CodesGet: Sized {
    fn get_from(
        handle: &metkit_sys::UniquePtr<metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<Self>;
}

/// Trait for types that can be written to a `CodesHandle`.
pub trait CodesSet {
    fn set_on(
        self,
        handle: std::pin::Pin<&mut metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<()>;
}

// ==================== CodesGet impls ====================

impl CodesGet for String {
    fn get_from(
        handle: &metkit_sys::UniquePtr<metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<Self> {
        handle.get_string(key).map_err(crate::Error::from)
    }
}

impl CodesGet for i64 {
    fn get_from(
        handle: &metkit_sys::UniquePtr<metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<Self> {
        handle.get_long(key).map_err(crate::Error::from)
    }
}

impl CodesGet for f64 {
    fn get_from(
        handle: &metkit_sys::UniquePtr<metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<Self> {
        handle.get_double(key).map_err(crate::Error::from)
    }
}

impl CodesGet for Vec<f64> {
    fn get_from(
        handle: &metkit_sys::UniquePtr<metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<Self> {
        handle.get_double_array(key).map_err(crate::Error::from)
    }
}

impl CodesGet for Vec<i64> {
    fn get_from(
        handle: &metkit_sys::UniquePtr<metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<Self> {
        handle.get_long_array(key).map_err(crate::Error::from)
    }
}

// ==================== CodesSet impls ====================

impl CodesSet for &str {
    fn set_on(
        self,
        handle: std::pin::Pin<&mut metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<()> {
        handle.set_string(key, self).map_err(crate::Error::from)
    }
}

impl CodesSet for String {
    fn set_on(
        self,
        handle: std::pin::Pin<&mut metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<()> {
        handle.set_string(key, &self).map_err(crate::Error::from)
    }
}

impl CodesSet for i64 {
    fn set_on(
        self,
        handle: std::pin::Pin<&mut metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<()> {
        handle.set_long(key, self).map_err(crate::Error::from)
    }
}

impl CodesSet for f64 {
    fn set_on(
        self,
        handle: std::pin::Pin<&mut metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<()> {
        handle.set_double(key, self).map_err(crate::Error::from)
    }
}

impl CodesSet for &[f64] {
    fn set_on(
        self,
        handle: std::pin::Pin<&mut metkit_sys::CodesHandleWrapper>,
        key: &str,
    ) -> crate::Result<()> {
        handle
            .set_double_array(key, self)
            .map_err(crate::Error::from)
    }
}
