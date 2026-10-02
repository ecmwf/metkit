/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>

#include "metkit/mars2grib/backend/concepts/quantile/quantileEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"


namespace metkit::mars2grib::backend::concepts_ {

/// Placeholder matcher until quantile variants and activation rules are defined.
template <class MarsDict_t, class OptDict_t>
std::size_t quantileMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    (void)mars;
    (void)opt;

    try {

        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        const auto marsType = get_or_throw<std::string>(mars, "type");
        const auto param    = get_or_throw<long>(mars, "param");

        if ( has<std::string>(mars, "quantile") && marsType != "sot") {
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
