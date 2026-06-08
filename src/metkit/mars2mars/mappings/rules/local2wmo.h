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

/// @file local2wmo.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/mappings/rules/common.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl {

/// @brief Assign `param`, and `scaleFactor` together.
template <class OutDict_t>
inline void setParamScale(OutDict_t& out, eckit::LocalConfiguration& misc, long param, double scaleFactor) {

    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        set_or_throw<long>(out, "param", param);
        set_or_throw<double>(misc, "scaleFactor", scaleFactor);
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to set param and/or scaleFactor in output dictionaries", Here()));
    }
}


/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t>
inline void convertLocal2WMO(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        const auto param = get_or_throw<long>(in, "param");

        switch (param) {
            // Cloud cover (0-1 -> %)
            case 164:
                return setParamScale(out, misc, 228164, 100.0);
            case 186:
                return setParamScale(out, misc, 3073, 100.0);
            case 187:
                return setParamScale(out, misc, 3074, 100.0);
            case 188:
                return setParamScale(out, misc, 3075, 100.0);

            // Precipitation (m -> kg m-2)
            case 143:
                return setParamScale(out, misc, 228143, 1000.0);
            case 228:
                return setParamScale(out, misc, 228228, 1000.0);

            // Runoff water (m -> kg m-2)
            case 205:
                return setParamScale(out, misc, 231002, 1000.0);

            // Snowfall water equivalent (m -> kg m-2)
            case 144:
                return setParamScale(out, misc, 228144, 10000.0);

            // Runoff and water cycle (m -> kg m-2)
            case 8:
                return setParamScale(out, misc, 231010, 1000.0);
            case 9:
                return setParamScale(out, misc, 231012, 1000.0);
            case 141:
                return setParamScale(out, misc, 231141, 1000.0);
            case 142:
                return setParamScale(out, misc, 3062, 1000.0);
            case 182:
                return setParamScale(out, misc, 260259, 1000.0);
            case 198:
                return setParamScale(out, misc, 160198, 1000.0);
            case 239:
                return setParamScale(out, misc, 231057, 1000.0);
            case 240:
                return setParamScale(out, misc, 231058, 1000.0);
            case 228216:
                return setParamScale(out, misc, 231001, 1000.0);

            // Albedo (0-1 -> %) (for ERA)
            case 15:
                return setParamScale(out, misc, 210199, 100.0);
            case 16:
                return setParamScale(out, misc, 210198, 100.0);
            case 17:
                return setParamScale(out, misc, 210261, 100.0);
            case 18:
                return setParamScale(out, misc, 210260, 100.0);
            case 32:
                return setParamScale(out, misc, 228032, 100.0);
            case 174:
                setParamScale(out, misc, 260509, 100.0);
                set_or_throw<long>(misc, "typeOfGeneratingProcess", 9);  // Climatological
                return;
            case 243:
                return setParamScale(out, misc, 260509, 100.0);
            case 210186:
                return setParamScale(out, misc, 210201, 100.0);
            case 210187:
                return setParamScale(out, misc, 210202, 100.0);
            case 210188:
                return setParamScale(out, misc, 210200, 100.0);
            case 210189:
                return setParamScale(out, misc, 210263, 100.0);
            case 210190:
                return setParamScale(out, misc, 210264, 100.0);
            case 210191:
                return setParamScale(out, misc, 210262, 100.0);

            // Water cycle (m -> kg m-2) (for ERA)
            case 44:
                return setParamScale(out, misc, 231003, 1000.0);
            case 45:
                return setParamScale(out, misc, 3099, 1000.0);
            case 228251:
                return setParamScale(out, misc, 231005, 1000.0);

            // Total column ozone: tco3 -> tcioz (kg m-2 -> DU, 1 DU = 2.1415e-5 kg m-2)
            case 206:
                return setParamScale(out, misc, 260132, 46698.05);

            // Time-mean rate (m s-1 -> kg m-2 s-1)
            case 172008:
                return setParamScale(out, misc, 235020, 1000.0);
            case 172009:
                return setParamScale(out, misc, 235021, 1000.0);
            case 172044:
                return setParamScale(out, misc, 235023, 1000.0);
            case 172045:
                return setParamScale(out, misc, 235024, 1000.0);
            case 172142:
                return setParamScale(out, misc, 235029, 1000.0);
            case 172143:
                return setParamScale(out, misc, 235030, 1000.0);
            case 172144:
                return setParamScale(out, misc, 235031, 1000.0);
            case 172182:
                return setParamScale(out, misc, 235043, 1000.0);
            case 172205:
                return setParamScale(out, misc, 235048, 1000.0);
            case 172228:
                return setParamScale(out, misc, 235055, 1000.0);
            case 235141:
                return setParamScale(out, misc, 235078, 1000.0);

            // Time-mean cloud cover and albedo (0-1 -> %)
            case 235186:
                return setParamScale(out, misc, 235108, 100.0);
            case 235187:
                return setParamScale(out, misc, 235109, 100.0);
            case 235188:
                return setParamScale(out, misc, 235110, 100.0);
            case 235243:
                return setParamScale(out, misc, 235263, 100.0);

            default:
                break;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to convert input dictionary in convertLocal2WMO", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl
