/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file waveDirectionGrid.h
/// @brief Resolve alternative misc inputs for a wave-direction grid.
///
/// Directions may be supplied as a real array or reconstructed from their
/// count. The scale factor is common to both representations and defaults to 2.
///

#pragma once

#include <vector>

#include "metkit/mars2grib/backend/misc-defaults/Access.h"
#include "metkit/mars2grib/backend/misc-defaults/Types.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults::rules {

struct WaveDirectionGrid {
    using value_type = WaveDirectionInput;
    static constexpr std::string_view name{"waveDirectionGrid"};
    static constexpr MiscPolicy policy{MiscPolicy::Mandatory};

    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<value_type> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());

        const auto scale = detail::read_optional<long>(
            misc, "scaleFactorOfWaveDirections", utils::profiling::callSite(cntx, Here()));
        const bool hasValues =
            detail::contains(misc, "waveDirections", utils::profiling::callSite(cntx, Here()));
        const bool hasCount = detail::contains(
            misc, "numberOfWaveDirections", utils::profiling::callSite(cntx, Here()));

        WaveDirectionInput input;
        input.scaleFactor = scale.value_or(2L);
        if (hasValues) {
            input.directions = detail::read_present_or_throw<std::vector<double>>(
                misc, "waveDirections", utils::profiling::callSite(cntx, Here()));
            ResolvedMisc<value_type> result{std::move(input), MiscSource::Explicit, "waveDirections"};
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        if (hasCount) {
            input.numberOfDirections = detail::read_present_or_throw<long>(
                misc, "numberOfWaveDirections", utils::profiling::callSite(cntx, Here()));
            ResolvedMisc<value_type> result{std::move(input), MiscSource::Explicit,
                                            "numberOfWaveDirections"};
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        throw utils::exceptions::Mars2GribMiscDefaultsException(
            "TODO: fallback not implemented for mandatory misc property `waveDirectionGrid`", Here());
    }
};

}  // namespace metkit::mars2grib::backend::misc_defaults::rules
