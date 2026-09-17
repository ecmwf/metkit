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
#include <memory>

// Utils
#include "eckit/geo/Grid.h"
#include "eckit/spec/Custom.h"
#include "metkit/mars2grib/backend/concepts/representation/representationEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/Profiling.h"

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
template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::size_t representationMatcherImpl(const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    try {
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        // This is used to fully delegate section3 setting to gridSpec
        if (get_or_throw<bool>(opt, "skipSection3", utils::profiling::callSite(cntx, Here()))) {
            const auto marsGrid = get_or_throw<std::string>(mars, "grid", utils::profiling::callSite(cntx, Here()));
            const auto gridType =
                std::unique_ptr<const eckit::geo::Grid>(eckit::geo::GridFactory::make_from_string(marsGrid))->type();

            if (gridType == "sh") {
                const std::size_t result = static_cast<std::size_t>(RepresentationType::DummySH);
                utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
            const std::size_t result = static_cast<std::size_t>(RepresentationType::Dummy);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        if (has(mars, "truncation", utils::profiling::callSite(cntx, Here()))) {
            const std::size_t result = static_cast<std::size_t>(RepresentationType::SphericalHarmonics);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        const auto marsGrid = get_or_throw<std::string>(mars, "grid", utils::profiling::callSite(cntx, Here()));
        const auto gridType = std::unique_ptr<const eckit::geo::Grid>(
                                  eckit::geo::GridFactory::build(eckit::spec::Custom{{"grid", marsGrid}}))
                                  ->type();
        if (gridType == "regular_gg") {
            const std::size_t result = static_cast<std::size_t>(RepresentationType::RegularGaussian);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        else if (gridType == "reduced_gg") {
            const std::size_t result = static_cast<std::size_t>(RepresentationType::ReducedGaussian);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        else if (gridType == "regular_ll") {
            const std::size_t result = static_cast<std::size_t>(RepresentationType::Latlon);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        else if (gridType == "ORCA") {
            const std::size_t result = static_cast<std::size_t>(RepresentationType::Orca);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        else if (gridType == "HEALPix") {
            const std::size_t result = static_cast<std::size_t>(RepresentationType::Healpix);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "Cannot match grid \"" + marsGrid + "\" with grid type \"" + gridType + "\"! ", Here());
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `representation` concept", Here()));
    }
}

template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::size_t representationMatcher(const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const std::size_t result = representationMatcherImpl(mars, opt, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::backend::concepts_
