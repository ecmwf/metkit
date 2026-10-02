/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>

#include "metkit/mars2grib/backend/concepts/quantile/quantileEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"


namespace metkit::mars2grib::backend::concepts_ {

/// The concept is active when the MARS request carries `quantile`, except for `type=sot`: its `quantile`
/// (`90-99:100`) holds the two percentiles of the reference period, see the `referencePeriod` concept.
template <class MarsDict_t, class OptDict_t>
std::size_t quantileMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    (void)opt;

    try {

        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        const auto marsType = get_or_throw<std::string>(mars, "type");

        if (has(mars, "quantile") && marsType != "sot") {
            return static_cast<std::size_t>(QuantileType::Default);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `quantile` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
