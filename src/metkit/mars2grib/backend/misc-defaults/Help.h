/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file Help.h
/// @brief Build structured misc help after a requirements-recording run.
///
/// This API consumes the physical misc keys recorded by a dictionary, maps them
/// to GRIB keys, groups multi-key representations, and evaluates defaults that
/// depend on the current MARS request and options.
///

#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/mars2grib/backend/misc-defaults/Registry.h"
#include "metkit/mars2grib/backend/misc-defaults/rules/bitsPerValue.h"
#include "metkit/mars2grib/backend/misc-defaults/rules/subSetTruncation.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults {

namespace detail {

template <class MarsDict_t, class OptDict_t, class Cntx_t>
using EffectiveDefaultFunction =
    std::optional<std::string> (*)(const MarsDict_t&, const OptDict_t&, Cntx_t&);

template <class MarsDict_t, class OptDict_t, class Cntx_t>
struct EffectiveDefaultRegistration {
    std::string_view gribKey;
    EffectiveDefaultFunction<MarsDict_t, OptDict_t, Cntx_t> evaluate;
};

template <class MarsDict_t, class OptDict_t, class Cntx_t>
const std::array<EffectiveDefaultRegistration<MarsDict_t, OptDict_t, Cntx_t>, 9>&
effective_default_registry() {
    static const std::array<EffectiveDefaultRegistration<MarsDict_t, OptDict_t, Cntx_t>, 9> registry{{
        {"pv", [](const MarsDict_t&, const OptDict_t&, Cntx_t&) {
             return std::optional<std::string>{"pvSize=137"};
         }},
        {"subCentre", [](const MarsDict_t&, const OptDict_t&, Cntx_t&) {
             return std::optional<std::string>{"0"};
         }},
        {"numberOfForecastsInEnsemble", [](const MarsDict_t&, const OptDict_t&, Cntx_t&) {
             return std::optional<std::string>{"51"};
         }},
        {"numberOfFrequencies", [](const MarsDict_t&, const OptDict_t&, Cntx_t&) {
             return std::optional<std::string>{"54"};
         }},
        {"shapeOfTheEarth", [](const MarsDict_t&, const OptDict_t&, Cntx_t&) {
             return std::optional<std::string>{"6"};
         }},
        {"scaledValuesOfWaveDirections", [](const MarsDict_t&, const OptDict_t&, Cntx_t&) {
             return std::optional<std::string>{"scaleFactorOfWaveDirections=2"};
         }},
        {"scaledValuesOfWaveFrequencies", [](const MarsDict_t&, const OptDict_t&, Cntx_t&) {
             return std::optional<std::string>{"scaleFactorOfWaveFrequencies=6"};
         }},
        {"bitsPerValue", [](const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
             return std::optional<std::string>{std::to_string(rules::BitsPerValue::default_value(
                 mars, opt, utils::profiling::callSite(cntx, Here())))};
         }},
        {"subSetJ", [](const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
             const long truncation = rules::SubSetTruncation::truncation(
                 mars, opt, utils::profiling::callSite(cntx, Here()));
             return std::optional<std::string>{
                 std::to_string(truncation >= 213L ? 20L : std::min(10L, truncation))};
         }}
    }};
    return registry;
}

template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::optional<std::string> effective_default(std::string_view gribKey, const MarsDict_t& mars,
                                            const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    for (const auto& entry : effective_default_registry<MarsDict_t, OptDict_t, Cntx_t>()) {
        if (entry.gribKey != gribKey) continue;
        auto result = entry.evaluate(mars, opt, utils::profiling::callSite(cntx, Here()));
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return std::nullopt;
}

}  // namespace detail

/// Build one help record for every GRIB key reached by recorded misc keys.
template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::vector<MiscHelp> describe_recorded_misc(const std::vector<std::string>& recordedMiscKeys,
                                             const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    try {
        std::vector<MiscHelp> result;
        for (const auto gribKey : grib_keys_for_misc_keys(recordedMiscKeys)) {
            MiscHelp help;
            help.gribKey = std::string(gribKey);
            help.effectiveDefault = detail::effective_default(
                gribKey, mars, opt, utils::profiling::callSite(cntx, Here()));

            for (const auto& registration : misc_registry) {
                if (registration.gribKey != gribKey) continue;

                const MiscPolicy inputPolicy = registration.policy(registration.miscKey);
                if (inputPolicy == MiscPolicy::Mandatory) help.policy = MiscPolicy::Mandatory;
                else if (inputPolicy == MiscPolicy::OptionalOverride && help.policy != MiscPolicy::Mandatory) {
                    help.policy = MiscPolicy::OptionalOverride;
                }

                const auto types = registration.dataTypes(registration.miscKey);
                if (registration.role == MiscInputRole::Modifier) {
                    help.modifiers.push_back(MiscInput{
                        registration.miscKey, types, registration.description(registration.miscKey)});
                    continue;
                }

                MiscAlternative* alternative = nullptr;
                for (auto& candidate : help.alternatives) {
                    if (candidate.description == registration.alternative) {
                        alternative = &candidate;
                        break;
                    }
                }
                if (alternative == nullptr) {
                    help.alternatives.push_back(MiscAlternative{{}, std::string(registration.alternative)});
                    alternative = &help.alternatives.back();
                }

                alternative->inputs.push_back(MiscInput{
                    registration.miscKey, types, registration.description(registration.miscKey)});
            }

            if (help.policy == MiscPolicy::Mandatory && !help.effectiveDefault) {
                throw utils::exceptions::Mars2GribMiscDefaultsException(
                    "TODO: effective fallback not implemented for mandatory GRIB property `" + help.gribKey + "`",
                    Here());
            }

            help.description = "Accepted misc representations for GRIB key `" + help.gribKey + "`.";
            result.push_back(std::move(help));
        }
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMiscDefaultsException(
            "Unable to describe recorded misc requirements", Here()));
    }
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::misc_defaults
