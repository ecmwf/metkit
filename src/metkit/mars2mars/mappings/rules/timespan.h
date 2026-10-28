// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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
#include "metkit/mars2mars/utils/outputSchedule.h"
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
template <class InDict_t, class OutDict_t, class OptDict_t>
inline void convertStepRangeToTimespan(const InDict_t& in, OutDict_t& out, const OptDict_t& opts) {
    using metkit::mars2mars::utils::dict_traits::get_opt;
    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::has;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {

        if (!has(in, "step")) {
            return;
        }

        // Read as a string first: string-based dictionaries (MarsRequest) cannot read a range as long
        const auto stepString  = get_opt<std::string>(in, "step");
        const std::string step = stepString ? *stepString : std::to_string(get_or_throw<long>(in, "step"));

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

        if (endStep == startStep) {
            if (endStep != 0) {
                const long marsParamId = get_or_throw<long>(in, "param");
                const bool canFixZeroAccumulation =
                    marsParamId == 162110 && get_or_throw<bool>(opts, "tryFixBadInput_ZeroAccumulation");

                if (!canFixZeroAccumulation) {
                    const std::string marsClass  = get_or_throw<std::string>(in, "class");
                    const std::string marsStream = get_or_throw<std::string>(in, "stream");
                    const std::string marsType   = get_or_throw<std::string>(in, "type");
                    throw Mars2marsGenericException(
                        "Invalid step range `" + step + "`: endStep == startStep (" + std::to_string(endStep) +
                            " == " + std::to_string(startStep) + ") is only allowed for step 0" +
                            " (param=" + std::to_string(marsParamId) + ", class=" + marsClass +
                            ", stream=" + marsStream + ", type=" + marsType + ")",
                        Here());
                }
            }

            set_or_throw<long>(out, "step", endStep);
            set_or_throw<std::string>(out, "timespan", "fs");
        }
        else {
            set_or_throw<long>(out, "step", endStep);
            set_or_throw<std::string>(out, "timespan", std::to_string(endStep - startStep) + "h");
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to apply step-range timespan rule in output dictionary", Here()));
    }
}

/// @brief Give the accumulations since the start of the forecast `timespan=fs`.
///
/// Legacy accumulations are indexed by their end step only, so after `convertStepRangeToTimespan` they
/// carry `timespan=none`. `fs` is only valid for accumulations: maxima, minima and other fixed-window
/// statistics are handled by `fixTimespanSinceLastPostProcessing` and by the table in `fixTimespan`.
template <class InDict_t, class OutDict_t>
inline void fixTimespanFS(const InDict_t& in, OutDict_t& out) {
    using metkit::mars2mars::utils::dict_traits::get_opt;
    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    // Accumulations and time integrals since the start of the forecast (GRIB1 names), and nothing else:
    // the "since previous post-processing" params (49, 201, 202, 228226, 228227) and the fixed-window
    // params (121, 122, 123, 228026-228028, 228222-228225) must not be in this list.
    static const std::unordered_set<long> paramsWithTimespanFS{
        8,       // sro
        9,       // ssro
        20,      // parcs
        44,      // es
        45,      // smlt
        47,      // dsrp
        50,      // lspf
        57,      // uvb
        58,      // par
        142,     // lsp
        143,     // cp
        144,     // sf
        145,     // bld
        146,     // sshf
        147,     // slhf
        169,     // ssrd
        175,     // strd
        176,     // ssr
        177,     // str
        178,     // tsr
        179,     // ttr
        180,     // ewss
        181,     // nsss
        182,     // e
        189,     // sund
        195,     // lgws
        196,     // mgws
        197,     // gwd
        205,     // ro
        208,     // tsrc
        209,     // ttrc
        210,     // ssrc
        211,     // strc
        212,     // tisr
        213,     // vimd
        228,     // tp
        228021,  // fdir
        228022,  // cdir
        228080,  // aco2nee
        228081,  // aco2gpp
        228082,  // aco2rec
        228129,  // ssrdc
        228130,  // strdc
        228216,  // fzra
        228251,  // pev
    };

    try {
        // No timespan without a step (e.g. monthly means indexed by fcmonth)
        const auto timespan = get_opt<std::string>(out, "timespan");
        if (!timespan) {
            return;
        }

        const long param = get_or_throw<long>(in, "param");

        const bool isTimespanNone      = *timespan == "none";
        const bool isTimespanFSAllowed = paramsWithTimespanFS.find(param) != paramsWithTimespanFS.end();

        if (isTimespanNone && isTimespanFSAllowed) {
            set_or_throw<std::string>(out, "timespan", "fs");
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to apply step 0 timespan fix in output dictionary", Here()));
    }
}

/// @brief Window of the "since previous post-processing" params on a single step.
///
/// 10fg (49), mx2t (201), mn2t (202), mxtpr (228226) and mntpr (228227) run from the previous output step
/// of the model to the current one, so their window is the output interval of the (class, stream, type,
/// time) context, taken from the output schedule (`mars2mars-output-schedule.yaml`, see `OutputSchedule`).
/// A step range (the data path) already carries the window and is left alone; only a single step with
/// `timespan=none` is handled here. Step 0 has no previous step and therefore no window: no such field
/// exists in the converted archive, so the field is flagged with `misc.dropField`. A context missing from
/// the schedule is an error: the window is never guessed, and `fs` is reserved for accumulations.
template <class InDict_t, class OutDict_t, class OptDict_t>
inline void fixTimespanSinceLastPostProcessing(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc,
                                               const OptDict_t& opts) {
    using metkit::mars2mars::utils::OutputSchedule;
    using metkit::mars2mars::utils::dict_traits::get_opt;
    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    static const std::unordered_set<long> sinceLastPostProcessing{49, 201, 202, 228226, 228227};

    try {
        const long param = get_or_throw<long>(in, "param");
        if (sinceLastPostProcessing.find(param) == sinceLastPostProcessing.end()) {
            return;
        }

        // No step (e.g. monthly means), or a step range that already gave the window
        const auto timespan = get_opt<std::string>(out, "timespan");
        if (!timespan || *timespan != "none") {
            return;
        }

        // step and time are strings in a MarsRequest and (usually) numbers in a typed dictionary
        const auto stepString = get_opt<std::string>(in, "step");
        const long step       = stepString ? std::stol(*stepString) : get_or_throw<long>(in, "step");
        const auto timeString = get_opt<std::string>(in, "time");
        const long hour       = timeString ? OutputSchedule::hourOfMarsTime(*timeString)
                                           : OutputSchedule::hourOfMarsTime(get_or_throw<long>(in, "time"));
        const OutputSchedule::Context context{get_or_throw<std::string>(in, "class"),
                                              get_or_throw<std::string>(out, "stream"),
                                              get_or_throw<std::string>(in, "type"), hour};

        const std::string scheduleFile = get_opt<std::string>(opts, "outputScheduleFile").value_or("");
        const auto window              = OutputSchedule::get(scheduleFile).window(context, step);

        if (!window) {
            misc.set("dropField", true);
            return;
        }

        set_or_throw<std::string>(out, "timespan", std::to_string(*window) + "h");
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Mars2marsGenericException(
            "Failed to apply the since-last-post-processing timespan rule in output dictionary", Here()));
    }
}

/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t, class OptDict_t>
inline void fixTimespan(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc, const OptDict_t& opts) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        // Handle step ranges (e.g. "0-6") and default timespan to "none" for single-value steps.
        convertStepRangeToTimespan(in, out, opts);

        // Accumulations since the start of the forecast
        fixTimespanFS(in, out);

        // "Since previous post-processing" params: the window is the output interval
        fixTimespanSinceLastPostProcessing(in, out, misc, opts);

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
