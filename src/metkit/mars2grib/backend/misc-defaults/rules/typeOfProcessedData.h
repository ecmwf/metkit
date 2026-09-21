/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file typeOfProcessedData.h
/// @brief Optional numeric or symbolic override for typeOfProcessedData.

#pragma once

#include <optional>
#include <string>

#include "metkit/mars2grib/backend/misc-defaults/Access.h"
#include "metkit/mars2grib/backend/misc-defaults/Types.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults::rules {

struct TypeOfProcessedData {
    using value_type = IntegerOrString;
    static constexpr std::string_view name{"typeOfProcessedData"};
    static constexpr MiscPolicy policy{MiscPolicy::OptionalOverride};

    template <class Misc, class Mars, class Opt, class Cntx>
    static std::optional<ResolvedMisc<value_type>> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        if (!detail::contains(misc, name, utils::profiling::callSite(cntx, Here()))) {
            utils::profiling::profileExitFunction(cntx, Here());
            return std::nullopt;
        }
        if (auto value = detail::try_read_present<long>(misc, name,
                utils::profiling::callSite(cntx, Here()))) {
            ResolvedMisc<value_type> result{*value, MiscSource::Explicit, name};
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        if (auto value = detail::try_read_present<std::string>(misc, name,
                utils::profiling::callSite(cntx, Here()))) {
            ResolvedMisc<value_type> result{*value, MiscSource::Explicit, name};
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        throw utils::exceptions::Mars2GribMiscDefaultsException(
            "Misc key `typeOfProcessedData` must be an integer or string", Here());
    }
};

}  // namespace metkit::mars2grib::backend::misc_defaults::rules
