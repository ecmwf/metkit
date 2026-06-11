//! Wind family detection via `metkit::mars::ParamID`.

/// A wind component family — U/V and VO/D parameter names.
///
/// Mirrors C++ `metkit::mars::ParamID::WindFamily`.
#[derive(Debug, Clone)]
pub struct WindFamily {
    pub u: String,
    pub v: String,
    pub vo: String,
    pub d: String,
}

/// Get all known wind families from metkit.
///
/// Mirrors C++ `metkit::mars::ParamID::getWindFamilies()`.
#[must_use]
pub fn wind_families() -> Vec<WindFamily> {
    let count = metkit_sys::wind_family_count();
    let mut families = Vec::with_capacity(count);
    for i in 0..count {
        // These only fail on out-of-bounds, which can't happen here
        families.push(WindFamily {
            u: metkit_sys::wind_family_u(i).unwrap_or_else(|_| String::new()),
            v: metkit_sys::wind_family_v(i).unwrap_or_else(|_| String::new()),
            vo: metkit_sys::wind_family_vo(i).unwrap_or_else(|_| String::new()),
            d: metkit_sys::wind_family_d(i).unwrap_or_else(|_| String::new()),
        });
    }
    families
}
