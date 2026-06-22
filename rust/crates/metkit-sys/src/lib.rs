//! FFI bindings to ECMWF metkit C++ library.
//!
//! Provides cxx bridge to:
//! - `MarsRequest` — MARS request parsing, access, modification
//! - `RequestEnvironment` — environment metadata
//! - `MarsLanguage` — keyword categorization

use bindman::track_cpp_api;

// Auto-generated metkit Error enum + From<cxx::Exception> impl
include!(concat!(env!("OUT_DIR"), "/metkit_exceptions.rs"));

#[track_cpp_api(
    ("metkit/mars/MarsRequest.h", class = "MarsRequest"),
    ("metkit/mars/MarsLanguage.h", class = "MarsLanguage"),
)]
#[cxx::bridge(namespace = "metkit_bridge")]
pub mod ffi {
    unsafe extern "C++" {
        include!("MetkitBridge.h");

        // ==================== MarsRequest ====================

        type MarsRequestWrapper;

        fn verb(self: &MarsRequestWrapper) -> String;
        fn has(self: &MarsRequestWrapper, key: &str) -> bool;
        fn values(self: &MarsRequestWrapper, key: &str) -> Result<Vec<String>>;
        fn get_first(self: &MarsRequestWrapper, key: &str) -> Result<String>;
        fn empty(self: &MarsRequestWrapper) -> bool;
        fn count(self: &MarsRequestWrapper) -> usize;
        fn matches(self: &MarsRequestWrapper, filter: &MarsRequestWrapper) -> bool;
        fn params(self: &MarsRequestWrapper) -> Vec<String>;

        // Mutation
        fn set_verb(self: Pin<&mut MarsRequestWrapper>, verb: &str);
        fn set_value_string(self: Pin<&mut MarsRequestWrapper>, key: &str, value: &str);
        fn set_values(self: Pin<&mut MarsRequestWrapper>, key: &str, values: Vec<String>);
        fn set_value_long(self: Pin<&mut MarsRequestWrapper>, key: &str, value: i64);
        fn unset_values(self: Pin<&mut MarsRequestWrapper>, key: &str);

        // Extract parameters by category (e.g. "postproc")
        fn extract(
            self: &MarsRequestWrapper,
            category: &str,
        ) -> Result<UniquePtr<MarsRequestWrapper>>;

        // Expansion
        fn expand(
            self: &MarsRequestWrapper,
            inherit: bool,
            strict: bool,
        ) -> Result<UniquePtr<MarsRequestWrapper>>;

        // Cross-crate ExternTypes from eckit-sys
        #[namespace = "eckit_bridge"]
        type StreamWrapper = eckit_sys::StreamWrapper;

        #[namespace = "eckit_bridge"]
        type DataHandleWrapper = eckit_sys::DataHandleWrapper;

        #[namespace = "eckit_bridge"]
        type ConfigWrapper = eckit_sys::ConfigWrapper;

        #[namespace = "eckit_bridge"]
        type MessageWrapper = eckit_sys::MessageWrapper;

        fn encode(self: &MarsRequestWrapper, stream: Pin<&mut StreamWrapper>) -> Result<()>;

        /// Create a `MarsRequestHandle` — DataHandle for Hermes protocol.
        fn make_handle(
            self: &MarsRequestWrapper,
            config: &ConfigWrapper,
        ) -> Result<UniquePtr<DataHandleWrapper>>;

        // Output
        fn to_json(self: &MarsRequestWrapper) -> Result<String>;
        fn dump(self: &MarsRequestWrapper) -> String;

        // Factories
        #[Self = "MarsRequestWrapper"]
        #[must_use]
        fn create(verb: &str) -> UniquePtr<MarsRequestWrapper>;

        /// Create a `MarsRequest` from a GRIB message (reads metadata keys).
        #[Self = "MarsRequestWrapper"]
        fn from_message(msg: &MessageWrapper) -> Result<UniquePtr<MarsRequestWrapper>>;

        /// Decode a `MarsRequest` from a stream.
        #[Self = "MarsRequestWrapper"]
        fn decode(stream: Pin<&mut StreamWrapper>) -> Result<UniquePtr<MarsRequestWrapper>>;

        // ==================== CodesHandle ====================

        type CodesHandleWrapper;

        // Query
        fn is_defined(self: &CodesHandleWrapper, key: &str) -> Result<bool>;
        fn is_missing(self: &CodesHandleWrapper, key: &str) -> Result<bool>;
        fn has(self: &CodesHandleWrapper, key: &str) -> Result<bool>;

        // Scalar get
        fn get_string(self: &CodesHandleWrapper, key: &str) -> Result<String>;
        fn get_long(self: &CodesHandleWrapper, key: &str) -> Result<i64>;
        fn get_double(self: &CodesHandleWrapper, key: &str) -> Result<f64>;

        // Array get
        fn get_double_array(self: &CodesHandleWrapper, key: &str) -> Result<Vec<f64>>;
        fn get_long_array(self: &CodesHandleWrapper, key: &str) -> Result<Vec<i64>>;

        // Scalar set
        fn set_string(self: Pin<&mut CodesHandleWrapper>, key: &str, value: &str) -> Result<()>;
        fn set_long(self: Pin<&mut CodesHandleWrapper>, key: &str, value: i64) -> Result<()>;
        fn set_double(self: Pin<&mut CodesHandleWrapper>, key: &str, value: f64) -> Result<()>;

        // Array set
        fn set_double_array(
            self: Pin<&mut CodesHandleWrapper>,
            key: &str,
            values: &[f64],
        ) -> Result<()>;

        // Missing
        fn set_missing(self: Pin<&mut CodesHandleWrapper>, key: &str) -> Result<()>;

        // Size and data
        fn value_count(self: &CodesHandleWrapper, key: &str) -> Result<usize>;
        fn message_size(self: &CodesHandleWrapper) -> Result<usize>;
        fn message_data(self: &CodesHandleWrapper) -> Result<&[u8]>;

        // Clone
        #[rust_name = "clone_handle"]
        fn clone(self: &CodesHandleWrapper) -> Result<UniquePtr<CodesHandleWrapper>>;

        // Factories — static methods on CodesHandleWrapper, called from Rust as
        // `ffi::CodesHandleWrapper::from_message(...)` etc.
        #[Self = "CodesHandleWrapper"]
        fn from_message(data: &[u8]) -> Result<UniquePtr<CodesHandleWrapper>>;
        #[Self = "CodesHandleWrapper"]
        fn from_file(path: &str) -> Result<UniquePtr<CodesHandleWrapper>>;
        #[Self = "CodesHandleWrapper"]
        fn from_file_at_offset(path: &str, offset: i64) -> Result<UniquePtr<CodesHandleWrapper>>;
        #[Self = "CodesHandleWrapper"]
        fn from_sample(sample: &str) -> Result<UniquePtr<CodesHandleWrapper>>;

        // ==================== HyperCube ====================

        type HyperCubeWrapper;

        fn size(self: &HyperCubeWrapper) -> usize;
        fn count(self: &HyperCubeWrapper) -> usize;
        fn count_vacant(self: &HyperCubeWrapper) -> usize;
        fn contains(self: &HyperCubeWrapper, request: &MarsRequestWrapper) -> Result<bool>;
        fn clear(self: Pin<&mut HyperCubeWrapper>, request: &MarsRequestWrapper) -> Result<bool>;
        fn field_ordinal(self: &HyperCubeWrapper, request: &MarsRequestWrapper) -> Result<usize>;

        #[Self = "HyperCubeWrapper"]
        fn create(request: &MarsRequestWrapper) -> Result<UniquePtr<HyperCubeWrapper>>;

        // ==================== ParsedRequests ====================

        type ParsedRequestsWrapper;
        fn count(self: &ParsedRequestsWrapper) -> usize;
        fn at(self: &ParsedRequestsWrapper, index: usize) -> Result<UniquePtr<MarsRequestWrapper>>;

        #[Self = "ParsedRequestsWrapper"]
        fn parse(input: &str, strict: bool) -> Result<UniquePtr<ParsedRequestsWrapper>>;

        /// Raw parse without verb validation — uses `MarsParser` directly.
        #[Self = "ParsedRequestsWrapper"]
        fn parse_raw(input: &str) -> Result<UniquePtr<ParsedRequestsWrapper>>;

        // ==================== RequestEnvironment ====================

        type RequestEnvironmentWrapper;

        #[Self = "RequestEnvironmentWrapper"]
        fn initialise(keys: Vec<String>, values: Vec<String>);

        #[Self = "RequestEnvironmentWrapper"]
        fn request() -> Result<UniquePtr<MarsRequestWrapper>>;

        // ==================== MarsLanguage ====================

        type MarsLanguageWrapper;

        fn sink_keywords(self: &MarsLanguageWrapper) -> Vec<String>;
        fn is_data(self: &MarsLanguageWrapper, keyword: &str) -> bool;

        #[Self = "MarsLanguageWrapper"]
        fn create(verb: &str) -> Result<UniquePtr<MarsLanguageWrapper>>;
    }
}

// Public re-exports
pub use cxx::{Exception, UniquePtr};
pub use ffi::*;
