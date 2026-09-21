/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file waveFrequencyGrid.h
/// @brief Resolve alternative misc inputs for a wave-frequency grid.
///
/// Frequencies may be supplied as a real array or reconstructed from the
/// existing four-key parameter group. The scale factor is common to both
/// representations and defaults to 6.
///

#pragma once

#include <string>
#include <vector>

#include "metkit/mars2grib/backend/misc-defaults/Access.h"
#include "metkit/mars2grib/backend/misc-defaults/Types.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults::rules {

struct WaveFrequencyGrid {
    using value_type = WaveFrequencyInput;
    static constexpr std::string_view name{"waveFrequencyGrid"};
    static constexpr MiscPolicy policy{MiscPolicy::Mandatory};

    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<value_type> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());

        const auto scale = detail::read_optional<long>(
            misc, "scaleFactorOfWaveFrequencies", utils::profiling::callSite(cntx, Here()));
        const bool hasValues =
            detail::contains(misc, "waveFrequencies", utils::profiling::callSite(cntx, Here()));
        const bool hasCount = detail::contains(
            misc, "numberOfWaveFrequencies", utils::profiling::callSite(cntx, Here()));
        const bool hasIndex = detail::contains(
            misc, "indexOfReferenceWaveFrequency", utils::profiling::callSite(cntx, Here()));
        const bool hasReference = detail::contains(
            misc, "referenceWaveFrequency", utils::profiling::callSite(cntx, Here()));
        const bool hasRatio = detail::contains(
            misc, "waveFrequencySpacingRatio", utils::profiling::callSite(cntx, Here()));

        WaveFrequencyInput input;
        input.scaleFactor = scale.value_or(6L);
        if (hasValues) {
            input.frequencies = detail::read_present_or_throw<std::vector<double>>(
                misc, "waveFrequencies", utils::profiling::callSite(cntx, Here()));
            ResolvedMisc<value_type> result{std::move(input), MiscSource::Explicit, "waveFrequencies"};
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        const int reconstructionFields = static_cast<int>(hasCount) + static_cast<int>(hasIndex) +
                                         static_cast<int>(hasReference) + static_cast<int>(hasRatio);
        if (reconstructionFields == 4) {
            input.numberOfFrequencies = detail::read_present_or_throw<long>(
                misc, "numberOfWaveFrequencies", utils::profiling::callSite(cntx, Here()));
            input.referenceIndex = detail::read_present_or_throw<long>(
                misc, "indexOfReferenceWaveFrequency", utils::profiling::callSite(cntx, Here()));
            input.referenceFrequency = detail::read_present_or_throw<double>(
                misc, "referenceWaveFrequency", utils::profiling::callSite(cntx, Here()));
            input.spacingRatio = detail::read_present_or_throw<double>(
                misc, "waveFrequencySpacingRatio", utils::profiling::callSite(cntx, Here()));
            ResolvedMisc<value_type> result{std::move(input), MiscSource::Explicit, "reconstruction"};
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        if (reconstructionFields != 0) {
            throw utils::exceptions::Mars2GribMiscDefaultsException(
                "The wave-frequency reconstruction representation is incomplete", Here());
        }

        throw utils::exceptions::Mars2GribMiscDefaultsException(
            "TODO: fallback not implemented for mandatory misc property `waveFrequencyGrid`", Here());
    }
};

}  // namespace metkit::mars2grib::backend::misc_defaults::rules
