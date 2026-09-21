/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file typeOfEnsembleForecast.h
/// @brief Resolve an optional override of the ensemble-type deduction.
#pragma once
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct TypeOfEnsembleForecast {
    using value_type = long;
    static constexpr std::string_view name{"typeOfEnsembleForecast"};
    static constexpr MiscPolicy policy{MiscPolicy::OptionalOverride};
    template <class Misc, class Mars, class Opt, class Cntx>
    static std::optional<ResolvedMisc<long>> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        return detail::resolve_scalar_optional<long>(misc, name, cntx);
    }
};
}
