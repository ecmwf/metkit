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
/// @file representationMatcher.h
/// @brief Entry-level matcher for the GRIB `representation` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// select the GRIB grid representation variant from MARS metadata.
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

// Utils
#include "eckit/geo/Grid.h"
#include "eckit/spec/Custom.h"
#include "metkit/mars2grib/backend/concepts/representation/representationEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `representation` concept variant.
///
/// Spherical harmonics are selected when `truncation` is present. Otherwise the
/// matcher builds the eckit geometry from MARS `grid` and maps the resulting
/// grid type onto the corresponding representation variant.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local representation variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If the grid cannot be resolved, is unsupported, or lower-level matcher
/// evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t representationMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        if (has(mars, "truncation")) {
            return static_cast<std::size_t>(RepresentationType::SphericalHarmonics);
        }

        const auto marsGrid = get_or_throw<std::string>(mars, "grid");
        const auto gridType = eckit::geo::GridFactory::build(eckit::spec::Custom{{"grid", marsGrid}})->type();
        if (gridType == "regular-gg") {
            return static_cast<std::size_t>(RepresentationType::RegularGaussian);
        }
        else if (gridType == "reduced-gg") {
            return static_cast<std::size_t>(RepresentationType::ReducedGaussian);
        }
        else if (gridType == "regular-ll") {
            return static_cast<std::size_t>(RepresentationType::Latlon);
        }
        else if (gridType == "ORCA") {
            return static_cast<std::size_t>(RepresentationType::Orca);
        }
        else if (gridType == "healpix") {
            return static_cast<std::size_t>(RepresentationType::Healpix);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "Cannot match grid \"" + marsGrid + "\" with grid type \"" + gridType + "\"! ", Here());
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `representation` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
