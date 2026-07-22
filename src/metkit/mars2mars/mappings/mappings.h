/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file all.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/mappings/rules/misc-params.h"
#include "metkit/mars2mars/mappings/rules/wave2oper.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

// All the rules
#include "metkit/mars2mars/mappings/rules/chemical.h"
#include "metkit/mars2mars/mappings/rules/ecc-1806.h"
#include "metkit/mars2mars/mappings/rules/local2wmo.h"
#include "metkit/mars2mars/mappings/rules/sfc2sol.h"
#include "metkit/mars2mars/mappings/rules/timespan.h"
#include "metkit/mars2mars/mappings/rules/wave2oper.h"
#include "metkit/mars2mars/mappings/rules/windspeed.h"

namespace metkit::mars2mars::rules {

/// @brief Apply all registered conversion rules and return a result object.
template <class InDict_t, class OutDict_t>
Mars2MarsResult<OutDict_t> convertAll(const InDict_t& in) {

    using metkit::mars2mars::utils::dict_traits::clone_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        std::unique_ptr<OutDict_t> out                  = clone_or_throw(in);
        std::unique_ptr<eckit::LocalConfiguration> misc = std::make_unique<eckit::LocalConfiguration>();

        // Apply all conversions in sequence
        impl::convertWave2Oper(in, *out, *misc);
        impl::convertECC1806(in, *out, *misc);
        impl::convertSFC2SOL(in, *out, *misc);
        impl::convertLocal2WMO(in, *out, *misc);
        impl::fixTimespan(in, *out, *misc);
        impl::fixWindspeed(in, *out, *misc);
        impl::convertChemical(in, *out, *misc);
        impl::convertMiscParams(in, *out, *misc);

        return Mars2MarsResult<OutDict_t>{std::move(*out), std::move(*misc)};
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to convertAll input dictionaries", Here()));
    }
}

}  // namespace metkit::mars2mars::rules
