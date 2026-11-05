//! Request environment — metadata sent with protocol requests.

use crate::error::Result;
use crate::request::MarsRequest;

/// Initialize the global request environment.
///
/// Must be called before using the Hermes protocol.
/// Typically called once at startup with client name and version.
///
/// # Example
///
/// ```ignore
/// metkit::initialize(&[
///     ("client", "mars-client-rs"),
///     ("version", "0.1.0"),
/// ]);
/// ```
pub fn initialize(env: &[(&str, &str)]) {
    let keys: Vec<String> = env.iter().map(|(k, _)| (*k).to_string()).collect();
    let values: Vec<String> = env.iter().map(|(_, v)| (*v).to_string()).collect();
    metkit_sys::RequestEnvironmentWrapper::initialise(keys, values);
}

/// Get the current environment as a `MarsRequest`.
pub fn environment_request() -> Result<MarsRequest> {
    let inner = metkit_sys::RequestEnvironmentWrapper::request().map_err(crate::Error::from)?;
    Ok(MarsRequest::from_raw(inner))
}
