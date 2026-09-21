/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file waveFrequencies.h
/// @brief Resolve an explicit array of wave frequencies.
#pragma once
#include <vector>
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct WaveFrequencies {
    using value_type = std::vector<double>;
    static constexpr std::string_view name{"waveFrequencies"};
    static constexpr MiscPolicy policy{MiscPolicy::Mandatory};
    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<value_type> resolve(const Misc& misc, const Mars& mars, const Opt& opt, Cntx& cntx) {
        return detail::resolve_scalar_default<value_type>(misc, mars, opt, name,
            [](const auto& m, const auto& o, auto& c) {
                return detail::fallback_not_implemented<value_type>(m, o, name, c);
            }, cntx);
    }
};
}
