//! MARS language — keyword categorization.

use crate::error::Result;

/// MARS language definition for a specific verb.
///
/// Provides keyword categorization (data, postproc, sink).
pub struct MarsLanguage {
    inner: metkit_sys::UniquePtr<metkit_sys::MarsLanguageWrapper>,
}

impl MarsLanguage {
    /// Create a language definition for a verb.
    pub fn new(verb: &str) -> Result<Self> {
        let inner = metkit_sys::MarsLanguageWrapper::create(verb).map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Get all sink keywords (target, fieldset, etc.).
    #[must_use]
    pub fn sink_keywords(&self) -> Vec<String> {
        self.inner.sink_keywords()
    }

    /// Check if a keyword is a data axis keyword.
    #[must_use]
    pub fn is_data(&self, keyword: &str) -> bool {
        self.inner.is_data(keyword)
    }
}
