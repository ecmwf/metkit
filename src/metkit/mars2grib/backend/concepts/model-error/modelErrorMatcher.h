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
/// @file modelErrorMatcher.h
/// @brief Entry-level matcher for the GRIB `modelError` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether the **modelError concept** is active for a MARS request.
///
/// The matcher follows the standard mars2grib matching contract:
/// - return a local concept variant index when the concept is active,
/// - return `compile_time_registry_engine::MISSING` when it is not active.
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of
/// `concepts` to avoid conflicts with the C++20 `concept` language
/// feature.
///
/// @ingroup mars2grib_backend_concepts
///

#pragma once

// System include
#include <cstddef>
#include <string>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/concepts/model-error/modelErrorEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t modelErrorMatcher(const MarsDict_t& mars, const OptDict_t& opt) {


    try {

        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        if (!has(mars, "type")) {
            throw metkit::mars2grib::utils::exceptions::Mars2GribMatcherException(
                "MARS key `type` is required to determine applicability of the `modelError` concept but is missing. "
                "This is a contract violation by the upstream tool that populates the MARS dictionary.",
                Here());
        }

        // Concept does not apply unless "type" is present and equals "eme"
        if ((get_or_throw<std::string>(mars, "type") != "eme" && get_or_throw<std::string>(mars, "type") != "me")) {
            return compile_time_registry_engine::MISSING;
        }

        // At this point the request is a model-error request: "number" is mandatory
        if (has(mars, "number")) {
            return static_cast<std::size_t>(ModelErrorType::ComponentIndex);
        }

        if (has(mars, "coeffindex")) {
            return static_cast<std::size_t>(ModelErrorType::FourierCoefficients);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        // Rethrow nested exceptions with a more specific message
        std::throw_with_nested(metkit::mars2grib::utils::exceptions::Mars2GribMatcherException(
            "An error occurred while matching the `modelError` concept. Check nested exception for details.", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
