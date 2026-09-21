/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file lengthOfTimeWindow.h
/// @brief Resolve an optional time-window length and normalize it to seconds.
#pragma once
#include <limits>
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct LengthOfTimeWindow {
    using value_type = long;
    static constexpr std::string_view name{"lengthOfTimeWindow"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};
    template <class Misc, class Mars, class Opt, class Cntx>
    static std::optional<ResolvedMisc<long>> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        auto value = detail::resolve_scalar_optional<long>(misc, name, cntx);
        if (value) {
            if (value->value > std::numeric_limits<long>::max() / 3600L ||
                value->value < std::numeric_limits<long>::min() / 3600L) {
                throw utils::exceptions::Mars2GribMiscDefaultsException(
                    "Misc key `lengthOfTimeWindow` overflows when converted to seconds", Here());
            }
            value->value *= 3600L;
        }
        return value;
    }
};
}
