/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file timeIncrementInSeconds.h
/// @brief Normalized optional misc time increment.

#pragma once

#include <optional>
#include <string>

#include "metkit/mars2grib/backend/misc-defaults/Access.h"
#include "metkit/mars2grib/backend/misc-defaults/Types.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults::rules {

struct TimeIncrementInSeconds {
    using value_type = TimeIncrement;
    static constexpr std::string_view name{"timeIncrementInSeconds"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};

    template <class Misc, class Mars, class Opt, class Cntx>
    static std::optional<ResolvedMisc<value_type>> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        if (!detail::contains(misc, name, utils::profiling::callSite(cntx, Here()))) {
            utils::profiling::profileExitFunction(cntx, Here());
            return std::nullopt;
        }

        long seconds = 0;
        if (auto value = detail::try_read_present<long>(misc, name,
                utils::profiling::callSite(cntx, Here()))) {
            seconds = *value;
        }
        else if (auto value = detail::try_read_present<std::string>(misc, name,
                     utils::profiling::callSite(cntx, Here()))) {
            std::size_t parsed = 0;
            try {
                seconds = std::stol(*value, &parsed);
            }
            catch (...) {
                std::throw_with_nested(utils::exceptions::Mars2GribMiscDefaultsException(
                    "Misc key `timeIncrementInSeconds` must contain an integer", Here()));
            }
            if (parsed != value->size()) {
                throw utils::exceptions::Mars2GribMiscDefaultsException(
                    "Misc key `timeIncrementInSeconds` must contain an integer", Here());
            }
        }
        else {
            throw utils::exceptions::Mars2GribMiscDefaultsException(
                "Misc key `timeIncrementInSeconds` must be an integer or string", Here());
        }
        if (seconds <= 0) {
            throw utils::exceptions::Mars2GribMiscDefaultsException(
                "Misc key `timeIncrementInSeconds` must be positive", Here());
        }
        ResolvedMisc<value_type> result{TimeIncrement{seconds}, MiscSource::Explicit, name};
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

}  // namespace metkit::mars2grib::backend::misc_defaults::rules
