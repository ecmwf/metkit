/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file waveFrequencySpacingRatio.h
/// @brief Resolve the wave-frequency reconstruction spacing ratio.
#pragma once
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct WaveFrequencySpacingRatio {
    using value_type = double;
    static constexpr std::string_view name{"waveFrequencySpacingRatio"};
    static constexpr MiscPolicy policy{MiscPolicy::Mandatory};
    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<double> resolve(const Misc& misc, const Mars& mars, const Opt& opt, Cntx& cntx) {
        return detail::resolve_scalar_default<double>(misc, mars, opt, name,
            [](const auto& m, const auto& o, auto& c) {
                return detail::fallback_not_implemented<double>(m, o, name, c);
            }, cntx);
    }
};
}
