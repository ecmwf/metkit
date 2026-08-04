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

/// @file timespan.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <unordered_set>

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/mappings/rules/common.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl {

/// @brief Assign `param`, and `timespan` together.
template <class OutDict_t>
inline void setParamTimespan(OutDict_t& out, long param, const std::string& timespan) {

    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        set_or_throw<long>(out, "param", param);
        set_or_throw<std::string>(out, "timespan", timespan);
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to set param and/or timespan in output dictionary", Here()));
    }
}

/// @brief Detect a `step` of the form "<startStep>-<endStep>" and rewrite
/// `step` + `timespan` on `out`. When `step` is a single value, default the
/// output `timespan` to "none" if it is not already set on `in`.
template <class InDict_t, class OutDict_t>
inline void convertStepRangeToTimespan(const InDict_t& in, OutDict_t& out) {
    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::has;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {

        if (!has(in, "step")) {
            return;
        }

        const std::string step = has<long>(in, "step") ? std::to_string(get_or_throw<long>(in, "step"))
                                                       : get_or_throw<std::string>(in, "step");

        // Strict range detection: "<digits>-<digits>"
        const auto dash        = step.find('-');
        const auto isAllDigits = [](std::string_view v) {
            return !v.empty() && std::all_of(v.begin(), v.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
        };
        const bool isRange = dash != std::string::npos && step.find('-', dash + 1) == std::string::npos &&
                             isAllDigits(std::string_view{step.data(), dash}) &&
                             isAllDigits(std::string_view{step.data() + dash + 1, step.size() - dash - 1});

        if (!isRange) {
            // Single-value step: leave step untouched. Default timespan to
            // "none" only when the input has no timespan yet.
            if (!has(in, "timespan")) {
                set_or_throw<std::string>(out, "timespan", "none");
            }
            return;
        }

        const long startStep = std::stol(std::string(step, 0, dash));
        const long endStep   = std::stol(std::string(step, dash + 1));

        if (endStep < startStep) {
            throw Mars2marsGenericException("Invalid step range `" + step + "`: endStep < startStep (" +
                                                std::to_string(endStep) + " < " + std::to_string(startStep) + ")",
                                            Here());
        }

        if (endStep == startStep && endStep != 0) {
            throw Mars2marsGenericException("Invalid step range `" + step + "`: endStep == startStep (" +
                                                std::to_string(endStep) + " == " + std::to_string(startStep) +
                                                ") is only allowed for step 0",
                                            Here());
        }

        set_or_throw<long>(out, "step", endStep);
        set_or_throw<std::string>(out, "timespan", std::to_string(endStep - startStep) + "h");
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to apply step-range timespan rule in output dictionary", Here()));
    }
}

/// @brief Fix timespan of statistical fields that have been wrongly encoded as instant at step 0.
template <class InDict_t, class OutDict_t>
inline void fixStep0Timespan(const InDict_t& in, OutDict_t& out) {
    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::has;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    static const std::unordered_set<long> paramIdsWithTimespan{
        8,      9,      20,     44,     45,     47,     49,     50,     57,     58,     121,    122,
        123,    142,    143,    144,    146,    145,    147,    169,    175,    176,    177,    178,
        179,    180,    181,    182,    189,    195,    196,    197,    201,    202,    205,    208,
        209,    210,    211,    212,    213,    228,    228021, 228022, 228080, 228081, 228082, 228129,
        228130, 228216, 228222, 228223, 228224, 228225, 228226, 228227, 228026, 228027, 228028, 228251};

    try {

        // This fix should only be applied if step == 0.
        // Note that when step is the range 0-0, timespan is already set to 0 in convertStepRangeToTimespan.
        if (has<long>(in, "step") && get_or_throw<long>(in, "step") != 0) {
            return;
        }

        const long paramId = get_or_throw<long>(in, "param");

        if (paramIdsWithTimespan.find(paramId) == paramIdsWithTimespan.end()) {
            return;
        }

        set_or_throw<std::string>(out, "timespan", "0h");
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to apply step 0 timespan fix in output dictionary", Here()));
    }
}

/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t>
inline void fixTimespan(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        // Handle step ranges (e.g. "0-6") and default timespan to "none" for single-value steps.
        convertStepRangeToTimespan(in, out);

        // Fix statistical fields that are wrongly encoded as instant fields at step 0.
        fixStep0Timespan(in, out);

        const auto param = get_or_throw<long>(in, "param");

        switch (param) {
            // Max 2t
            case 121:
                return setParamTimespan(out, 237167, "6h");
            case 228026:
                return setParamTimespan(out, 237167, "3h");
            case 201:
                return set_or_throw<long>(out, "param", 237167);

            // Max 6h CAPE
            case 228035:
                return setParamTimespan(out, 237117, "6h");

            // Max 6h CAPES
            case 228036:
                return setParamTimespan(out, 237321, "6h");

            // Max precipitation
            case 228222:
                return setParamTimespan(out, 237055, "3h");
            case 228224:
                return setParamTimespan(out, 237055, "6h");
            case 228226:
                return set_or_throw<long>(out, "param", 237055);

            // Max wind gust
            case 123:
                return setParamTimespan(out, 237318, "6h");
            case 228028:
                return setParamTimespan(out, 237318, "3h");
            case 49:
                return set_or_throw<long>(out, "param", 237318);

            // Mean flash density ?
            case 228051:
                return setParamTimespan(out, 235326, "1h");
            case 228057:
                return setParamTimespan(out, 235326, "3h");
            case 228058:
                return setParamTimespan(out, 235326, "6h");

            // Min 2t
            case 122:
                return setParamTimespan(out, 238167, "6h");
            case 228027:
                return setParamTimespan(out, 238167, "3h");
            case 202:
                return set_or_throw<long>(out, "param", 238167);

            // Min precipitation
            case 228223:
                return setParamTimespan(out, 238055, "3h");
            case 228225:
                return setParamTimespan(out, 238055, "6h");
            case 228227:
                return set_or_throw<long>(out, "param", 238055);

            // Most frequent precipitation type
            case 260320:
                return setParamTimespan(out, 260683, "1h");
            case 260321:
                return setParamTimespan(out, 260683, "3h");
            case 260339:
                return setParamTimespan(out, 260683, "6h");

            // Most severe precipitation type
            case 260318:
                return setParamTimespan(out, 260682, "1h");
            case 260319:
                return setParamTimespan(out, 260682, "3h");
            case 260338:
                return setParamTimespan(out, 260682, "6h");

            default:
                break;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException("Failed to convert input dictionary in fixTimespan", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl
