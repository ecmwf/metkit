/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file normalization.h
/// @brief Utilities for input dictionary sanitization and normalization.
///
/// This header provides "just-in-time" sanitization routines used to ensure
/// input dictionaries conform to backend expectations before the resolution
/// phase begins.
///
/// @ingroup mars2grib_frontend_normalization
///
#pragma once

// Project includes
#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/enableOptions.h"

namespace metkit::mars2grib::frontend::normalization {

///
/// @brief Conditionally sanitizes a dictionary based on runtime options.
///
/// This function implements a **pass-through or transform** pattern designed
/// to minimize unnecessary copies. If sanitization is required by the provided
/// options, the transformed data is stored in a caller-provided scratch
/// buffer, and a reference to that buffer is returned. Otherwise, the
/// original dictionary is returned as-is.
///
/// @tparam MiscDict_t Type of the dictionary to be sanitized
/// @tparam OptDict_t  Type of the options dictionary driving the logic
///
/// @param[in]  miscDict Original dictionary to evaluate
/// @param[in]  optDict  Configuration/Options dict used to determine policy
/// @param[out] scratch  Buffer used to store the sanitized result if needed
///
/// @return A const reference to either the original or the sanitized dictionary
///
template <class MiscDict_t, class OptDict_t>
const MiscDict_t& normalize_MiscDict_if_enabled(const MiscDict_t& miscDict, const OptDict_t& optDict,
                                                const eckit::Value& language, MiscDict_t& scratch) {

    using metkit::mars2grib::utils::normalizeMiscEnabled;

    // TODO: Implement sanitization trigger logic based on optDict settings
    if (normalizeMiscEnabled(optDict)) {

        // [Development Stub]
        // Example: logic to prune illegal keys or normalize units
        // scratch = perform_transform(miscDict);

        return scratch;
    }

    // Default path: zero-copy pass-through
    return miscDict;
}

}  // namespace metkit::mars2grib::frontend::normalization