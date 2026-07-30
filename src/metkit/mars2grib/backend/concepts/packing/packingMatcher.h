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
/// @file packingMatcher.h
/// @brief Entry-level matcher for the GRIB `packing` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// select the GRIB data packing variant requested by MARS metadata.
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

// Project includes
#include "metkit/mars2grib/backend/concepts/packing/packingEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `packing` concept variant.
///
/// The matcher maps the MARS `packing` keyword onto the corresponding local
/// packing concept variant.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local packing variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If the `packing` keyword is missing, has an unsupported value, or lower-level
/// matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t packingMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::exceptions::Mars2GribMatcherException;

        const auto& packing = get_or_throw<std::string>(mars, "packing");
        if (packing == "simple") {
            return static_cast<std::size_t>(PackingType::Simple);
        }
        if (packing == "ccsds") {
            return static_cast<std::size_t>(PackingType::Ccsds);
        }
        if (packing == "complex") {
            return static_cast<std::size_t>(PackingType::SpectralComplex);
        }

        throw Mars2GribMatcherException{"Unknown value \"" + packing + "\" for mars keyword \"packing\"!", Here()};
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `packing` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
