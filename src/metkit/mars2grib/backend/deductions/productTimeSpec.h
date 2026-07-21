/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
#pragma once

#include <exception>
#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpecResolver.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

/// Resolve the canonical ProductTimeSpec from MARS, parameter, and option
/// dictionaries. All dictionary access is performed by the fully templated
/// ProductTimeSpecInput through utils::dict_traits.
template <class MarsDict_t, class ParDict_t, class OptDict_t>
product_time_spec::ProductTimeSpec resolve_ProductTimeSpec_or_throw(
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing,
    const MarsDict_t& mars,
    const ParDict_t& par,
    const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const product_time_spec::ProductTimeSpecInput<MarsDict_t, ParDict_t, OptDict_t>
            input{innerMostTypeOfStatisticalProcessing, mars, par, opt};

        auto result = product_time_spec::resolve_ProductTimeSpecInput_or_throw(input);

        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`ProductTimeSpec` resolved from input dictionaries: "} +
                   result.to_json();
        }());

        return result;
    } catch (...) {
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `ProductTimeSpec` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions
