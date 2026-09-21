/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file pv.h
/// @brief Resolve alternative misc representations of the GRIB PV array.
///
/// The canonical PV input can be supplied directly through `pv` or indirectly
/// through `pvSize`. The alternatives are mutually exclusive. When neither is
/// supplied, the effective default is `pvSize=137`.
///

#pragma once

#include <vector>

#include "metkit/mars2grib/backend/misc-defaults/Access.h"
#include "metkit/mars2grib/backend/misc-defaults/Types.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults::rules {

struct Pv {
    using value_type = PvArrayInput;
    static constexpr std::string_view name{"pv"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};

    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<value_type> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());

        const bool hasPv = detail::contains(misc, "pv", utils::profiling::callSite(cntx, Here()));
        const bool hasPvSize =
            detail::contains(misc, "pvSize", utils::profiling::callSite(cntx, Here()));

        if (hasPv && hasPvSize) {
            throw utils::exceptions::Mars2GribMiscDefaultsException(
                "Misc keys `pv` and `pvSize` are mutually exclusive", Here());
        }

        if (hasPv) {
            PvArrayInput input;
            input.values = detail::read_present_or_throw<std::vector<double>>(
                misc, "pv", utils::profiling::callSite(cntx, Here()));
            input.size = static_cast<long>(input.values->size());
            ResolvedMisc<value_type> result{std::move(input), MiscSource::Explicit, "pv"};
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        const long size = hasPvSize
                              ? detail::read_present_or_throw<long>(
                                    misc, "pvSize", utils::profiling::callSite(cntx, Here()))
                              : 137L;
        ResolvedMisc<value_type> result{PvArrayInput{std::nullopt, size},
                                        hasPvSize ? MiscSource::Explicit : MiscSource::Default, "pvSize"};
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

}  // namespace metkit::mars2grib::backend::misc_defaults::rules
