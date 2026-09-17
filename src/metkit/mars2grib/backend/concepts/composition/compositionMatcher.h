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
/// @file compositionMatcher.h
/// @brief Entry-level matcher for the GRIB `composition` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether chemical or aerosol composition metadata is active for a
/// MARS request.
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
#include "metkit/mars2grib/backend/concepts/composition/compositionEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/Profiling.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `composition` concept variant.
///
/// The concept is active as `CompositionType::Chem` when `chem` is present,
/// and as `CompositionType::Aerosol` when `wavelength` is present.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local composition variant index, or
/// `compile_time_registry_engine::MISSING` when no composition metadata is active.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::size_t compositionMatcherImpl(const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());

    try {

        const auto matchAny = [&cntx](auto&&... args) {
            return metkit::mars2grib::util::param_matcher::matchAny(args..., cntx);
        };
        using metkit::mars2grib::util::param_matcher::range;
        using metkit::mars2grib::utils::dict_traits::get_opt;
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;
        using metkit::mars2grib::utils::exceptions::Mars2GribMatcherException;

        const auto param = get_or_throw<long>(mars, "param", utils::profiling::callSite(cntx, Here()));

        // TODO: This is the range for CAMS, there are some unmapped parameters that may need to be supported for ERA6,
        // etc.
        if (param < 400000 || param >= 500000) {
            const std::size_t result = compile_time_registry_engine::MISSING;
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        const auto chem          = get_or_throw<long>(mars, "chem", utils::profiling::callSite(cntx, Here()));
        const auto hasWavelength = has(mars, "wavelength", utils::profiling::callSite(cntx, Here()));

        if (hasWavelength) {
            if (matchAny(param, 457000)) {
                if (matchAny(chem, range(900, 914), 918, 922, 923, range(933, 936))) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::AerosolOptical);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 458000, 459000, 460000, 461000, 462000, 472000)) {
                if (matchAny(chem, 922)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::AerosolOptical);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
        }
        else {
            if (matchAny(param, 401000)) {
                if (matchAny(chem, range(900, 916))) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Aerosol);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
                if (matchAny(chem, 2, 3, range(5, 24), range(26, 30), range(32, 50), 52, 53, range(55, 58),
                             range(63, 80), 82, 83, 85, 86, range(99, 101), 107, 112, 159, 161, 169, range(173, 178),
                             range(186, 204), 222, range(224, 231), 233, 311, 359, 404, 917)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Chem);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 402000)) {
                if (matchAny(chem, range(900, 917), 924)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Aerosol);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
                if (matchAny(chem, range(2, 24), range(26, 30), range(32, 50), 52, 53, range(55, 59), range(63, 80), 82,
                             83, 85, 86, range(99, 101), 107, 112, 118, 159, 161, 169, range(173, 178), range(186, 204),
                              222, range(224, 230), 233, 236, 311)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Chem);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 406000, 407000, 410000, 411000, 451000)) {
                if (matchAny(chem, range(901, 916))) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Aerosol);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 453000)) {
                if (matchAny(chem, range(901, 916), 922)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Aerosol);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 400000)) {
                if (matchAny(chem, range(929, 931))) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Aerosol);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 444000)) {
                if (matchAny(chem, 6, 8, 13, 15, 17, 19, 26, 27, 33)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Chem);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 445000)) {
                if (matchAny(chem, 6, 8, 13, 15, 17, 19, 27, 33, 236)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Chem);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 479000)) {
                if (matchAny(chem, 404)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::Chem);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
            else if (matchAny(param, 469000)) {
                if (matchAny(chem, 2, 5, 9, 10, 12, 16, 18, 19, 42, range(45, 48), 52, 99, 100, 129, 224, 226, 233, 311,
                              933, 934)) {
                    const std::size_t result = static_cast<std::size_t>(CompositionType::ChemicalSource);
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
            }
        }

        throw Mars2GribMatcherException(
            "compositionMatcher: matching logic is not implemented for param=" + std::to_string(param) +
                ", chem=" + std::to_string(chem) + ", hasWavelength=" + (hasWavelength ? "true" : "false"),
            Here());
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `composition` concept", Here()));
    }
}

template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::size_t compositionMatcher(const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const std::size_t result = compositionMatcherImpl(mars, opt, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::backend::concepts_
