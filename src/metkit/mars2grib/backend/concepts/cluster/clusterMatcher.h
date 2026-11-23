// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file clusterMatcher.h
/// @brief Entry-level matcher for the GRIB `cluster` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether cluster-product metadata is active for a request.
///
/// The matcher follows the standard mars2grib matching contract:
/// - return a local concept variant index when the concept is active,
/// - return `compile_time_registry_engine::MISSING` when it is not active,
/// - wrap runtime failures as nested `Mars2GribMatcherException` instances.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System include
#include <cstddef>
#include <exception>
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/cluster/clusterEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `cluster` concept variant.
///
/// The concept is active for cluster-product MARS `type` values, i.e.
/// cluster mean (`cm`) and cluster representative (`cr`).
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local variant index for `ClusterType::Rectangular`, or
/// `compile_time_registry_engine::MISSING` when the concept is inactive.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t clusterMatcher(const MarsDict_t& mars, const OptDict_t& opt) {


    try {

        using metkit::mars2grib::utils::dict_traits::get_or_throw;


        const auto& type = get_or_throw<std::string>(mars, "type");

        if (type == "cm" ||  // Cluster Mean
            type == "cr"     // Cluster Representative
        ) {
            return static_cast<std::size_t>(ClusterType::Rectangular);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `cluster` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
