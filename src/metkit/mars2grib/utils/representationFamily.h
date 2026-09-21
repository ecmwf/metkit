/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file representationFamily.h
/// @brief Resolve the semantic spatial representation family.
///
/// This utility deliberately distinguishes only spherical-harmonic and gridded
/// fields. Matching a concrete gridded representation remains the responsibility
/// of the representation concept.
///

#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <string_view>

#include "eckit/geo/Grid.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::utils {

enum class RepresentationFamily : std::uint8_t {
    SphericalHarmonics,
    Gridded
};

inline constexpr std::string_view representation_family_name(RepresentationFamily family) noexcept {
    return family == RepresentationFamily::SphericalHarmonics ? "sphericalHarmonics" : "gridded";
}

/// Resolve whether a request is spherical-harmonic or gridded.
///
/// When section 3 is externally managed, the family is inferred from the MARS
/// grid specification. Otherwise, the presence of MARS `truncation` identifies
/// spherical harmonics and no concrete grid lookup is performed here.
template <class MarsDict_t, class OptDict_t, class Cntx_t>
RepresentationFamily resolve_representation_family_or_throw(const MarsDict_t& mars, const OptDict_t& opt,
                                                             Cntx_t& cntx) {
    profiling::profileEnterFunction(cntx, Here());

    using dict_traits::get_or_throw;
    using dict_traits::has;
    using exceptions::Mars2GribGenericException;

    try {
        RepresentationFamily result = RepresentationFamily::Gridded;
        const bool skipSection3 =
            get_or_throw<bool>(opt, "skipSection3", profiling::callSite(cntx, Here()));

        if (skipSection3) {
            const std::string grid =
                get_or_throw<std::string>(mars, "grid", profiling::callSite(cntx, Here()));
            const std::unique_ptr<const eckit::geo::Grid> gridSpec(
                eckit::geo::GridFactory::make_from_string(grid));
            result = gridSpec->type() == "sh" ? RepresentationFamily::SphericalHarmonics
                                               : RepresentationFamily::Gridded;
        }
        else if (has(mars, "truncation", profiling::callSite(cntx, Here()))) {
            result = RepresentationFamily::SphericalHarmonics;
        }

        profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribGenericException("Unable to resolve representation family", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::utils
