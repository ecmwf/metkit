//! MARS request — builder and immutable validated request.

use crate::error::Result;

/// Builder for constructing a [`MarsRequest`].
///
/// # Example
///
/// ```ignore
/// let request = MarsRequestBuilder::new("retrieve")
///     .with("class", "od")
///     .with("step", "0/to/12/by/3")
///     .build();
/// ```
pub struct MarsRequestBuilder {
    inner: metkit_sys::UniquePtr<metkit_sys::MarsRequestWrapper>,
}

impl MarsRequestBuilder {
    /// Create a new builder with a verb.
    #[must_use]
    pub fn new(verb: &str) -> Self {
        Self {
            inner: metkit_sys::request_create(verb),
        }
    }

    /// Create a builder from an existing request (copy its fields).
    #[must_use]
    pub fn from(request: &MarsRequest) -> Self {
        // Rebuild by copying verb + all params
        let mut builder = Self::new(&request.verb());
        for key in request.params() {
            if let Ok(values) = request.values(&key) {
                for val in &values {
                    builder.inner.pin_mut().set_value_string(&key, val);
                }
            }
        }
        builder
    }

    /// Change the verb.
    #[must_use]
    pub fn verb(mut self, verb: &str) -> Self {
        self.inner.pin_mut().set_verb(verb);
        self
    }

    /// Set a single value for a key. Last write wins.
    #[must_use]
    pub fn with(mut self, key: &str, value: &str) -> Self {
        self.inner.pin_mut().set_value_string(key, value);
        self
    }

    /// Set multiple values for a key. Last write wins.
    #[must_use]
    pub fn with_values(mut self, key: &str, values: &[&str]) -> Self {
        let vec: Vec<String> = values.iter().map(|v| (*v).to_string()).collect();
        self.inner.pin_mut().set_values(key, vec);
        self
    }

    /// Remove a key.
    #[must_use]
    pub fn without(mut self, key: &str) -> Self {
        self.inner.pin_mut().unset_values(key);
        self
    }

    /// Build the request. Stores values as-is without expansion.
    #[must_use]
    pub fn build(self) -> MarsRequest {
        MarsRequest { inner: self.inner }
    }
}

/// An immutable, validated MARS request.
///
/// Created via [`MarsRequestBuilder::build()`] or [`str::parse()`].
/// All values have been expanded and validated by metkit.
pub struct MarsRequest {
    inner: metkit_sys::UniquePtr<metkit_sys::MarsRequestWrapper>,
}

// SAFETY: The underlying C++ `MarsRequest` is accessed through `&mut self`
// only on mutation paths and is otherwise immutable once built — same
// rationale as `eckit::Config` / `eckit::DataHandle`. `Sync` follows
// because all public `&self` methods read immutable state with no interior
// mutability on the C++ side.
#[allow(clippy::non_send_fields_in_send_ty)]
unsafe impl Send for MarsRequest {}
unsafe impl Sync for MarsRequest {}

impl MarsRequest {
    /// Create from a raw metkit-sys wrapper (for internal use).
    pub(crate) const fn from_raw(
        inner: metkit_sys::UniquePtr<metkit_sys::MarsRequestWrapper>,
    ) -> Self {
        Self { inner }
    }

    /// The request verb (e.g. `"retrieve"`, `"archive"`).
    #[must_use]
    pub fn verb(&self) -> String {
        self.inner.verb()
    }

    /// Check if a key exists.
    #[must_use]
    pub fn has(&self, key: &str) -> bool {
        self.inner.has(key)
    }

    /// Get all values for a key.
    pub fn values(&self, key: &str) -> Result<Vec<String>> {
        self.inner.values(key).map_err(crate::Error::from)
    }

    /// Get the first value for a key.
    pub fn get(&self, key: &str) -> Result<String> {
        self.inner.get_first(key).map_err(crate::Error::from)
    }

    /// Whether the request has no parameters.
    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.inner.empty()
    }

    /// Number of parameters.
    #[must_use]
    pub fn count(&self) -> usize {
        self.inner.count()
    }

    /// Check if this request matches a filter.
    #[must_use]
    pub fn matches(&self, filter: &Self) -> bool {
        self.inner.matches(&filter.inner)
    }

    /// Get all parameter names.
    #[must_use]
    pub fn params(&self) -> Vec<String> {
        self.inner.params()
    }

    /// Expand the request via `MarsLanguage::expand()`.
    ///
    /// Resolves date ranges, parameter aliases, etc.
    pub fn expand(&self, inherit: bool, strict: bool) -> Result<Self> {
        let inner = self
            .inner
            .expand(inherit, strict)
            .map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Extract parameters by category (e.g. `"postproc"`).
    ///
    /// Returns a new request containing only the extracted parameters.
    /// Mirrors C++ `MarsRequest::extract()`.
    pub fn extract(&self, category: &str) -> Result<Self> {
        let inner = self.inner.extract(category).map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Serialize to JSON.
    pub fn to_json(&self) -> Result<String> {
        self.inner.to_json().map_err(crate::Error::from)
    }

    /// Dump as formatted text.
    #[must_use]
    pub fn dump(&self) -> String {
        self.inner.dump()
    }

    /// Encode this request to a stream (C++ wire format).
    ///
    /// Calls C++ `MarsRequest::encode(Stream&)` via `operator<<`.
    pub fn encode(&self, stream: &mut dyn eckit::Stream) -> crate::Result<()> {
        self.inner
            .encode(stream.as_sys_mut())
            .map_err(crate::Error::from)
    }

    /// Decode a request from a stream (C++ wire format).
    ///
    /// Calls C++ `MarsRequest(Stream&)` constructor.
    pub fn decode(stream: &mut dyn eckit::Stream) -> crate::Result<Self> {
        let inner = metkit_sys::request_decode(stream.as_sys_mut()).map_err(crate::Error::from)?;
        Ok(Self { inner })
    }

    /// Access the underlying metkit-sys wrapper (for FFI interop with other -sys crates).
    #[must_use]
    pub fn as_sys(&self) -> &metkit_sys::MarsRequestWrapper {
        &self.inner
    }
}

/// Parsed requests — parse once, iterate by index.
///
/// # Example
///
/// ```ignore
/// let parsed = metkit::parse("retrieve, class=od\nlist, class=od", false)?;
/// for request in parsed.iter() {
///     let request = request?;
///     println!("{}", request.verb());
/// }
/// ```
pub struct ParsedRequests {
    inner: metkit_sys::UniquePtr<metkit_sys::ParsedRequestsWrapper>,
}

impl ParsedRequests {
    /// Number of parsed requests.
    #[must_use]
    pub fn count(&self) -> usize {
        self.inner.count()
    }

    /// Get request at index.
    pub fn at(&self, index: usize) -> Result<MarsRequest> {
        let inner = self.inner.at(index).map_err(crate::Error::from)?;
        Ok(MarsRequest::from_raw(inner))
    }

    /// Iterate over all parsed requests.
    #[must_use]
    pub const fn iter(&self) -> ParsedRequestsIter<'_> {
        ParsedRequestsIter {
            parsed: self,
            index: 0,
        }
    }
}

/// Iterator over parsed requests.
pub struct ParsedRequestsIter<'a> {
    parsed: &'a ParsedRequests,
    index: usize,
}

impl Iterator for ParsedRequestsIter<'_> {
    type Item = Result<MarsRequest>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index >= self.parsed.count() {
            return None;
        }
        let result = self.parsed.at(self.index);
        self.index += 1;
        Some(result)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let remaining = self.parsed.count() - self.index;
        (remaining, Some(remaining))
    }
}

impl ExactSizeIterator for ParsedRequestsIter<'_> {}

impl<'a> IntoIterator for &'a ParsedRequests {
    type Item = Result<MarsRequest>;
    type IntoIter = ParsedRequestsIter<'a>;

    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

/// Parse MARS requests from input text (with verb validation).
///
/// Mirrors C++ `MarsRequest::parse(istream, strict)`.
pub fn parse(input: &str, strict: bool) -> Result<ParsedRequests> {
    let inner = metkit_sys::parse_requests(input, strict).map_err(crate::Error::from)?;
    Ok(ParsedRequests { inner })
}

/// Tokenize MARS requests from input text (no verb validation).
///
/// Mirrors C++ `MarsParser(istream).parse()`.
/// Accepts any verb including `"environ"`.
pub fn tokenize(input: &str) -> Result<ParsedRequests> {
    let inner = metkit_sys::parse_requests_raw(input).map_err(crate::Error::from)?;
    Ok(ParsedRequests { inner })
}

impl std::fmt::Display for MarsRequest {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.inner.dump())
    }
}

impl eckit::StreamWrite for MarsRequest {
    // Trait fixes the error type to eckit::Error; bypass crate::Error wrapping.
    fn write_to(&self, stream: &mut dyn eckit::Stream) -> eckit::Result<()> {
        self.inner
            .encode(stream.as_sys_mut())
            .map_err(eckit::Error::from)
    }
}

impl TryFrom<&eckit::Message> for MarsRequest {
    type Error = crate::Error;

    fn try_from(msg: &eckit::Message) -> crate::Result<Self> {
        let inner = metkit_sys::request_from_message(msg.as_sys()).map_err(crate::Error::from)?;
        Ok(Self { inner })
    }
}
