//! MARS request — typestate-guarded: raw vs expanded, tracked by the type system.
//!
//! A [`MarsRequest`] is either:
//!
//! - [`MarsRequest<Raw>`] (the default) — mutable, values stored as-is, not yet
//!   validated by the MARS language, or
//! - [`MarsRequest<Expanded>`] — produced by metkit's own expansion
//!   ([`MarsRequest::expand`], [`parse`], [`str::parse`]) or by the explicit
//!   trust assertion [`MarsRequest::from_trusted`].
//!
//! APIs that require expanded values (e.g. `HyperCube::new`) take
//! `&MarsRequest<Expanded>`, so passing an unexpanded request is a compile
//! error. Mirrors the typestate style of `eckit::DataHandle<Closed>`.

use std::marker::PhantomData;

use crate::error::Result;

mod sealed {
    pub trait Sealed {}
}

/// State of a MARS request. Sealed — only [`Raw`] and [`Expanded`].
pub trait RequestState: sealed::Sealed {}

/// Values stored as-is; not yet validated/expanded by the MARS language.
pub struct Raw;
impl sealed::Sealed for Raw {}
impl RequestState for Raw {}

/// Expanded and validated by the MARS language (dates resolved, ranges
/// enumerated, aliases canonicalized, defaults inherited).
pub struct Expanded;
impl sealed::Sealed for Expanded {}
impl RequestState for Expanded {}

/// A MARS request, parameterized by its expansion state.
///
/// Mirrors the mutable C++ `metkit::mars::MarsRequest`. `MarsRequest` with no
/// type argument means `MarsRequest<Raw>`.
///
/// # Example
///
/// ```ignore
/// // Build a raw request, then expand it — the only way to get <Expanded>
/// let mut request = MarsRequest::new("retrieve");
/// request.set("class", "od");
/// request.set("date", 20260713);            // i64
/// request.set("step", ["0", "6", "12"]);    // multiple values
/// let expanded = request.expand(true, false)?;
///
/// // Parsing expands (C++ MarsRequest::parse runs MarsExpansion)
/// let request: MarsRequest<Expanded> = "retrieve, class=od, date=-1".parse()?;
///
/// // Trusted input — syntax-checked only, caller vouches for validity
/// let trusted = MarsRequest::from_trusted("retrieve, class=od, date=20260713")?;
///
/// // Editing an expanded request demotes it back to Raw
/// let mut edited = expanded.into_raw();
/// edited.set("expver", "0002");
/// let expanded = edited.expand(true, false)?;
/// ```
pub struct MarsRequest<S: RequestState = Raw> {
    inner: metkit_sys::UniquePtr<metkit_sys::MarsRequestWrapper>,
    _state: PhantomData<S>,
}

// SAFETY: `UniquePtr` gives this struct exclusive ownership of the C++
// `MarsRequest`, which holds its parameters by value (no shared state, no
// thread-locals). All mutation goes through `&mut self`
// (`Pin<&mut MarsRequestWrapper>`), so Rust's aliasing rules guarantee
// exclusive access during mutation — `Send` is sound. All `&self` methods
// map to `const` C++ methods with no interior mutability, so concurrent
// shared reads are safe — `Sync` follows. Same rationale as `eckit::Config`
// / `eckit::DataHandle`.
#[allow(clippy::non_send_fields_in_send_ty)]
unsafe impl<S: RequestState> Send for MarsRequest<S> {}
unsafe impl<S: RequestState> Sync for MarsRequest<S> {}

/// A value assignable to a [`MarsRequest`] key — a single string, an integer,
/// or a collection of strings.
///
/// Sealed: implemented for `&str`, `String`, `&String`, `i64`, `Vec<String>`,
/// `Vec<&str>`, `&[&str]`, `&[String]`, and `[&str; N]` / `&[&str; N]`.
///
/// For iterators, collect first: `.collect::<Vec<String>>()`. For a
/// `&Vec<String>`, pass `v.as_slice()`.
pub trait MarsValue: sealed::Sealed {
    #[doc(hidden)]
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str);
}

impl sealed::Sealed for &str {}
impl MarsValue for &str {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        request.inner.pin_mut().set_value_string(key, self);
    }
}

impl sealed::Sealed for String {}
impl MarsValue for String {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        request.inner.pin_mut().set_value_string(key, &self);
    }
}

impl sealed::Sealed for &String {}
impl MarsValue for &String {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        request.inner.pin_mut().set_value_string(key, self);
    }
}

impl sealed::Sealed for i64 {}
impl MarsValue for i64 {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        request.inner.pin_mut().set_value_long(key, self);
    }
}

impl sealed::Sealed for Vec<String> {}
impl MarsValue for Vec<String> {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        request.inner.pin_mut().set_values(key, self);
    }
}

impl sealed::Sealed for Vec<&str> {}
impl MarsValue for Vec<&str> {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        let values: Vec<String> = self.into_iter().map(str::to_string).collect();
        request.inner.pin_mut().set_values(key, values);
    }
}

impl sealed::Sealed for &[&str] {}
impl MarsValue for &[&str] {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        let values: Vec<String> = self.iter().map(|v| (*v).to_string()).collect();
        request.inner.pin_mut().set_values(key, values);
    }
}

impl sealed::Sealed for &[String] {}
impl MarsValue for &[String] {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        request.inner.pin_mut().set_values(key, self.to_vec());
    }
}

impl<const N: usize> sealed::Sealed for [&str; N] {}
impl<const N: usize> MarsValue for [&str; N] {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        self.as_slice().set_on(request, key);
    }
}

impl<const N: usize> sealed::Sealed for &[&str; N] {}
impl<const N: usize> MarsValue for &[&str; N] {
    fn set_on(self, request: &mut MarsRequest<Raw>, key: &str) {
        self.as_slice().set_on(request, key);
    }
}

impl MarsRequest<Raw> {
    /// Create a new raw request with a verb (e.g. `"retrieve"`).
    #[must_use]
    pub fn new(verb: &str) -> Self {
        Self {
            inner: metkit_sys::MarsRequestWrapper::create(verb),
            _state: PhantomData,
        }
    }

    /// Set the value(s) for a key. Last write wins.
    ///
    /// Accepts single strings, `i64`, and collections of strings — see
    /// [`MarsValue`].
    pub fn set(&mut self, key: &str, value: impl MarsValue) {
        value.set_on(self, key);
    }

    /// Remove a key.
    pub fn unset(&mut self, key: &str) {
        self.inner.pin_mut().unset_values(key);
    }

    /// Change the verb.
    pub fn set_verb(&mut self, verb: &str) {
        self.inner.pin_mut().set_verb(verb);
    }

    /// Decode a request from a stream (C++ wire format).
    ///
    /// Calls C++ `MarsRequest(Stream&)` constructor. The wire carries no
    /// expansion provenance, so the result is conservatively `Raw`.
    pub fn decode(stream: &mut dyn eckit::Stream) -> crate::Result<Self> {
        let inner = metkit_sys::MarsRequestWrapper::decode(stream.as_sys_mut())
            .map_err(crate::Error::from)?;
        Ok(Self {
            inner,
            _state: PhantomData,
        })
    }
}

impl MarsRequest<Expanded> {
    /// Parse trusted input as an already-expanded request.
    ///
    /// Runs only the MARS parser (syntax check, no verb validation) — **no
    /// language expansion or validation**. The caller asserts the values are
    /// already valid and canonical; an invalid request surfaces as a runtime
    /// error in downstream C++ instead of at this boundary.
    ///
    /// Prefer [`str::parse()`] unless you fully control the input.
    ///
    /// Errors if the input is not exactly one syntactically valid request.
    pub fn from_trusted(input: &str) -> Result<Self> {
        let parsed = tokenize(input)?;
        match parsed.count() {
            1 => {
                let MarsRequest { inner, .. } = parsed.at(0)?;
                Ok(Self {
                    inner,
                    _state: PhantomData,
                })
            }
            n => Err(crate::Error::Other(format!(
                "expected exactly one MARS request, found {n}"
            ))),
        }
    }
}

impl<S: RequestState> MarsRequest<S> {
    /// Create from a raw metkit-sys wrapper (for internal use).
    /// The caller chooses the state and is responsible for its truth.
    pub(crate) const fn from_raw(
        inner: metkit_sys::UniquePtr<metkit_sys::MarsRequestWrapper>,
    ) -> Self {
        Self {
            inner,
            _state: PhantomData,
        }
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

    /// Check if this request matches a filter (any state).
    #[must_use]
    pub fn matches<F: RequestState>(&self, filter: &MarsRequest<F>) -> bool {
        self.inner.matches(&filter.inner)
    }

    /// Get all parameter names.
    #[must_use]
    pub fn params(&self) -> Vec<String> {
        self.inner.params()
    }

    /// Expand the request via `MarsLanguage::expand()`.
    ///
    /// Resolves date ranges, parameter aliases, etc. This is the guarded
    /// [`Raw`] → [`Expanded`] transition; re-expanding an already expanded
    /// request is legal and idempotent.
    pub fn expand(&self, inherit: bool, strict: bool) -> Result<MarsRequest<Expanded>> {
        let inner = self
            .inner
            .expand(inherit, strict)
            .map_err(crate::Error::from)?;
        Ok(MarsRequest {
            inner,
            _state: PhantomData,
        })
    }

    /// Re-tag as [`Raw`] to edit the request. Free — no FFI call.
    ///
    /// Any edit invalidates expansion, so mutation lives on
    /// [`MarsRequest<Raw>`]; re-expand with [`MarsRequest::expand`] after
    /// editing.
    #[must_use]
    pub fn into_raw(self) -> MarsRequest<Raw> {
        let Self { inner, .. } = self;
        MarsRequest {
            inner,
            _state: PhantomData,
        }
    }

    /// Extract parameters by category (e.g. `"postproc"`).
    ///
    /// Returns a new request containing only the extracted parameters,
    /// keeping the expansion state. Mirrors C++ `MarsRequest::extract()`.
    pub fn extract(&self, category: &str) -> Result<Self> {
        let inner = self.inner.extract(category).map_err(crate::Error::from)?;
        Ok(Self {
            inner,
            _state: PhantomData,
        })
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

    /// Access the underlying metkit-sys wrapper (for FFI interop with other -sys crates).
    #[must_use]
    pub fn as_sys(&self) -> &metkit_sys::MarsRequestWrapper {
        &self.inner
    }
}

impl<S: RequestState> Clone for MarsRequest<S> {
    /// Rebuild by copying verb + all params. State-preserving.
    fn clone(&self) -> Self {
        let mut inner = metkit_sys::MarsRequestWrapper::create(&self.verb());
        for key in self.params() {
            if let Ok(values) = self.values(&key) {
                inner.pin_mut().set_values(&key, values);
            }
        }
        Self {
            inner,
            _state: PhantomData,
        }
    }
}

impl std::str::FromStr for MarsRequest<Expanded> {
    type Err = crate::Error;

    /// Parse a single MARS request, expanding and validating it (the C++
    /// `MarsRequest::parse` runs `MarsExpansion` internally).
    ///
    /// Errors if the input contains zero or more than one request — use
    /// [`parse`]/[`tokenize`] for multi-request text.
    fn from_str(s: &str) -> Result<Self> {
        let parsed = parse(s, false)?;
        match parsed.count() {
            1 => parsed.at(0),
            n => Err(crate::Error::Other(format!(
                "expected exactly one MARS request, found {n}"
            ))),
        }
    }
}

impl<S: RequestState> std::fmt::Display for MarsRequest<S> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.inner.dump())
    }
}

impl<S: RequestState> eckit::StreamWrite for MarsRequest<S> {
    // Trait fixes the error type to eckit::Error; bypass crate::Error wrapping.
    fn write_to(&self, stream: &mut dyn eckit::Stream) -> eckit::Result<()> {
        self.inner
            .encode(stream.as_sys_mut())
            .map_err(eckit::Error::from)
    }
}

impl TryFrom<&eckit::Message> for MarsRequest<Raw> {
    type Error = crate::Error;

    /// Build a request from a GRIB message's metadata keys.
    ///
    /// GRIB-derived requests are not language-expanded, hence `Raw`.
    fn try_from(msg: &eckit::Message) -> crate::Result<Self> {
        let inner = metkit_sys::MarsRequestWrapper::from_message(msg.as_sys())
            .map_err(crate::Error::from)?;
        Ok(Self {
            inner,
            _state: PhantomData,
        })
    }
}

/// Parsed requests — parse once, iterate by index.
///
/// The state parameter records how the requests were produced:
/// [`parse`] expands, so it yields `ParsedRequests<Expanded>`;
/// [`tokenize`] does not, so it yields `ParsedRequests<Raw>`.
///
/// # Example
///
/// ```ignore
/// let parsed = metkit::parse("retrieve, class=od\nlist, class=od", false)?;
/// for request in parsed.iter() {
///     let request = request?;   // MarsRequest<Expanded>
///     println!("{}", request.verb());
/// }
/// ```
pub struct ParsedRequests<S: RequestState> {
    inner: metkit_sys::UniquePtr<metkit_sys::ParsedRequestsWrapper>,
    _state: PhantomData<S>,
}

impl<S: RequestState> ParsedRequests<S> {
    /// Number of parsed requests.
    #[must_use]
    pub fn count(&self) -> usize {
        self.inner.count()
    }

    /// Get request at index.
    pub fn at(&self, index: usize) -> Result<MarsRequest<S>> {
        let inner = self.inner.at(index).map_err(crate::Error::from)?;
        Ok(MarsRequest::from_raw(inner))
    }

    /// Iterate over all parsed requests.
    #[must_use]
    pub const fn iter(&self) -> ParsedRequestsIter<'_, S> {
        ParsedRequestsIter {
            parsed: self,
            index: 0,
        }
    }
}

/// Iterator over parsed requests.
pub struct ParsedRequestsIter<'a, S: RequestState> {
    parsed: &'a ParsedRequests<S>,
    index: usize,
}

impl<S: RequestState> Iterator for ParsedRequestsIter<'_, S> {
    type Item = Result<MarsRequest<S>>;

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

impl<S: RequestState> ExactSizeIterator for ParsedRequestsIter<'_, S> {}

impl<'a, S: RequestState> IntoIterator for &'a ParsedRequests<S> {
    type Item = Result<MarsRequest<S>>;
    type IntoIter = ParsedRequestsIter<'a, S>;

    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

/// Parse MARS requests from input text, expanding and validating them.
///
/// Mirrors C++ `MarsRequest::parse(istream, strict)`, which runs
/// `MarsExpansion` internally — the results are [`Expanded`].
pub fn parse(input: &str, strict: bool) -> Result<ParsedRequests<Expanded>> {
    let inner =
        metkit_sys::ParsedRequestsWrapper::parse(input, strict).map_err(crate::Error::from)?;
    Ok(ParsedRequests {
        inner,
        _state: PhantomData,
    })
}

/// Tokenize MARS requests from input text (no expansion, no verb validation).
///
/// Mirrors C++ `MarsParser(istream).parse()` — the results are [`Raw`].
/// Accepts any verb including `"environ"`.
pub fn tokenize(input: &str) -> Result<ParsedRequests<Raw>> {
    let inner = metkit_sys::ParsedRequestsWrapper::parse_raw(input).map_err(crate::Error::from)?;
    Ok(ParsedRequests {
        inner,
        _state: PhantomData,
    })
}
