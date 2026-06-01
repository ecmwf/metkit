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
/// @file iterationMatcher.h
/// @brief Entry-level matcher for the GRIB `iteration` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether the **iteration concept** is active for a MARS request.
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

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/concepts/iteration/iterationEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t iterationMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(mars, "iteration")) {
        return static_cast<size_t>(IterationType::Default);
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_
