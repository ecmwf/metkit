/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file ProductTimeSpecInputStateAndMakers.h
 * @brief Intermediate extraction state and make-functions for ProductTimeSpecInput.
 *
 * This header decomposes ProductTimeSpecInput extraction into small,
 * independently documentable make-functions. Each function owns one local piece
 * of normalization or validation and returns one typed fragment of the final
 * input state.
 *
 * The extracted pieces are assembled into one intermediate
 * `ProductTimeSpecInputState`, which can then be consumed by a thin
 * ProductTimeSpecInput constructor. This keeps the public input class focused on
 * immutable state storage while the extraction pipeline remains explicit and
 * readable.
 *
 * Error model:
 *
 * - each local `make...` function adds a narrow, key- or fragment-specific
 *   extraction context;
 * - `makeProductTimeSpecInputState_or_throw(...)` adds the stable outer
 *   "failed to extract normalized ProductTimeSpec input" context.
 */

#pragma once

#include <optional>
#include <string>

#include "eckit/types/Date.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputCommon.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputTemporalParsing.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputDictionaryAccess.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputSourceLanguage.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputOptions.h"

namespace metkit::mars2grib::product_time_spec::input_detail::state_detail {

/**
 * @brief Mandatory normalized MARS product context.
 *
 * The ProductTimeSpec resolver requires lowercase `class`, `stream`, and
 * `type` even when a particular branch consumes only a subset of them.
 */
struct MarsContext {
    std::string marsClass{};
    std::string marsStream{};
    std::string marsType{};
    long marsParamId{-1};  // parameter ID from the mars dictionary.
};

/**
 * @brief Optional normalized reference-anchor year/month pair.
 *
 * The pair is validated as one indivisible source: both fields are either
 * present together or absent together.
 */
struct YearMonthAnchor {
    std::optional<long> year{};
    std::optional<long> month{};
};

/**
 * @brief Normalized source representation of the MARS `timespan` key.
 *
 * The pair stores both the structural source category and the optional positive
 * duration payload used when the category is `Duration`.
 */
struct TimespanInfo {
    TimespanKind timespanKind{TimespanKind::Missing};
    std::optional<long> timespanInSeconds{};
};

/**
 * @brief Complete intermediate extraction state for ProductTimeSpecInput.
 *
 * This aggregate mirrors the final ProductTimeSpecInput stored members. It
 * exists only as a staging object so the extraction pipeline can complete
 * before the final object is materialized.
 */
struct ProductTimeSpecInputState {
    std::optional<eckit::Date> marsDate{};
    std::optional<eckit::Time> marsTime{};
    std::optional<eckit::Date> marsHdate{};
    std::optional<long> marsYear{};
    std::optional<long> marsMonth{};
    std::optional<long> stepInSeconds{};
    TimespanKind timespanKind{TimespanKind::Missing};
    std::optional<long> timespanInSeconds{};
    ParsedStatTypeBlocks stattypeBlocks{};
    std::string marsClass{};
    std::string marsStream{};
    std::string marsType{};
    long marsParamId{-1};  // parameter ID from the mars dictionary.
    std::optional<long> timeIncrementInSeconds{};
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing{
        tables::TypeOfStatisticalProcessing::Missing};
    ProductTimeSpecOptions options{};
};

}  // namespace metkit::mars2grib::product_time_spec::input_detail::state_detail

namespace metkit::mars2grib::product_time_spec::input_detail {

/**
 * @brief Build the complete normalized ProductTimeSpec option snapshot.
 *
 * This is a thin context-adding wrapper around `parseOptions(...)`.
 *
 * @param opt Option dictionary.
 * @return Fully normalized ProductTimeSpecOptions snapshot.
 * @throws Mars2GribGenericException on option parsing or validation failure.
 */
template <class OptDict_t>
inline ProductTimeSpecOptions makeOptions(const OptDict_t& opt) {
    try {
        return parseOptions(opt);
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract ProductTimeSpec options",
            Here()));
    }
}

/**
 * @brief Build the mandatory lowercase MARS context triple.
 *
 * The returned context contains the normalized `class`, `stream`, and `type`
 * values required by later ProductTimeSpec shape and increment rules.
 *
 * @param mars MARS dictionary.
 * @return Normalized mandatory MARS context.
 * @throws Mars2GribGenericException when a mandatory key is missing or has the
 *         wrong type.
 */
template <class MarsDict_t>
inline state_detail::MarsContext makeMarsContext(const MarsDict_t& mars) {
    try {
        return state_detail::MarsContext{
            mandatoryString(mars, "class"),
            mandatoryString(mars, "stream"),
            mandatoryString(mars, "type"),
            mandatoryLong(mars, "param")};
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract mandatory MARS context `class`/`stream`/`type`",
            Here()));
    }
}

/**
 * @brief Build the optional normalized `date` source.
 *
 * @param mars MARS dictionary.
 * @return Normalized direct label date, or `std::nullopt` when absent.
 * @throws Mars2GribGenericException on unsupported representation or invalid
 *         date value.
 */
template <class MarsDict_t>
inline std::optional<eckit::Date> makeMarsDate(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    try {
        if (!has(mars, "date")) {
            return std::nullopt;
        }
        if (auto value = get_opt<long>(mars, "date")) {
            return parseDateLong(*value, "date");
        }
        if (auto value = get_opt<std::string>(mars, "date")) {
            return parseDateString(*value, "date");
        }
        throw Mars2GribGenericException("Unsupported type for `date`", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `date`",
            Here()));
    }
}

/**
 * @brief Build the optional normalized `time` source.
 *
 * @param mars MARS dictionary.
 * @return Normalized direct label time, or `std::nullopt` when absent.
 * @throws Mars2GribGenericException on unsupported representation or invalid
 *         time value.
 */
template <class MarsDict_t>
inline std::optional<eckit::Time> makeMarsTime(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    try {
        if (!has(mars, "time")) {
            return std::nullopt;
        }
        if (auto value = get_opt<long>(mars, "time")) {
            return parseTimeLong(*value, "time");
        }
        if (auto value = get_opt<std::string>(mars, "time")) {
            return parseTimeString(*value, "time");
        }
        throw Mars2GribGenericException("Unsupported type for `time`", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `time`",
            Here()));
    }
}


/**
 * @brief Build the optional normalized `hdate` source.
 *
 * @param mars MARS dictionary.
 * @return Normalized hindcast/reforecast date, or `std::nullopt` when absent.
 * @throws Mars2GribGenericException on unsupported representation or invalid
 *         date value.
 */
template <class MarsDict_t>
inline std::optional<eckit::Date> makeMarsHdate(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    try {
        if (!has(mars, "hdate")) {
            return std::nullopt;
        }
        if (auto value = get_opt<long>(mars, "hdate")) {
            return parseDateLong(*value, "hdate");
        }
        if (auto value = get_opt<std::string>(mars, "hdate")) {
            return parseDateString(*value, "hdate");
        }
        throw Mars2GribGenericException("Unsupported type for `hdate`", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `hdate`",
            Here()));
    }
}


/**
 * @brief Build the validated optional `year`/`month` reference anchor pair.
 *
 * Validation ensures:
 *
 * - both fields are present together or absent together;
 * - a present month lies in `[1,12]`;
 * - the first day of the resulting month is a valid `eckit::Date`.
 *
 * @param mars MARS dictionary.
 * @return Validated optional reference-anchor pair.
 * @throws Mars2GribGenericException on incomplete or invalid year/month input.
 */
template <class MarsDict_t>
inline state_detail::YearMonthAnchor makeMarsYearMonth(const MarsDict_t& mars) {
    try {
        state_detail::YearMonthAnchor result;
        result.year = optionalLong(mars, "year");
        result.month = optionalLong(mars, "month");

        if (result.year.has_value() != result.month.has_value()) {
            throw Mars2GribGenericException(
                "`year` and `month` must be both present or both absent",
                Here());
        }
        if (result.month && (*result.month < 1 || *result.month > 12)) {
            throw Mars2GribGenericException("`month` must be in [1,12]", Here());
        }
        if (result.year) {
            try {
                (void)eckit::Date(*result.year, *result.month, 1);
            } catch (...) {
                throw Mars2GribGenericException(
                    "Invalid `year`/`month` reference anchor",
                    Here());
            }
        }

        return result;
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `year`/`month` reference anchor",
            Here()));
    }
}


/**
 * @brief Build the optional normalized `step` in seconds.
 *
 * String-valued step carries explicit units; integer-valued step is interpreted
 * as hours. The current supported ProductTimeSpec domain accepts only
 * non-negative, whole-hour-aligned positive steps.
 *
 * @param mars MARS dictionary.
 * @return Normalized step in seconds, or `std::nullopt` when absent.
 * @throws Mars2GribGenericException on unsupported representation or unsupported
 *         step value.
 */
template <class MarsDict_t>
inline std::optional<long> makeStepInSeconds(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    try {
        if (!has(mars, "step")) {
            return std::nullopt;
        }

        long seconds = 0;
        if (auto value = get_opt<std::string>(mars, "step")) {
            seconds = parseDurationStringSeconds(*value, "step");
        } else if (auto value = get_opt<long>(mars, "step")) {
            seconds = checkedHoursToSeconds(*value, "step");
        } else {
            throw Mars2GribGenericException("Unsupported type for `step`", Here());
        }

        if (seconds < 0) {
            throw Mars2GribGenericException("`step` must be non-negative", Here());
        }
        if (seconds > 0 && seconds % 3600L != 0) {
            throw Mars2GribGenericException(
                "Positive sub-hourly or non-hour-aligned `step` is recognized but unsupported",
                Here());
        }

        return seconds;
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `step`",
            Here()));
    }
}


/**
 * @brief Build the normalized `timespan` representation pair.
 *
 * The function resolves the structural source representation of `timespan` into
 * a `TimespanKind` plus an optional strictly positive duration payload when the
 * source is duration-valued.
 *
 * Compatibility policy for non-enumerated positive integer-hour durations is
 * applied using the already-normalized option snapshot.
 *
 * @param mars MARS dictionary.
 * @param options Previously extracted option snapshot.
 * @return Normalized timespan representation.
 * @throws Mars2GribGenericException on unsupported, contradictory, or invalid
 *         `timespan` input.
 */
template <class MarsDict_t>
inline state_detail::TimespanInfo makeTimespanInfo(
    const MarsDict_t& mars,
    const ProductTimeSpecOptions& options) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    try {
        state_detail::TimespanInfo result;

        if (!has(mars, "timespan")) {
            result.timespanKind = TimespanKind::Missing;
            return result;
        }

        if (auto value = get_opt<std::string>(mars, "timespan")) {
            const std::string normalized = lower(*value);
            if (normalized == "none") {
                result.timespanKind = TimespanKind::None;
                return result;
            }
            if (normalized == "fs" || normalized == "from-start" || normalized == "fromstart") {
                result.timespanKind = TimespanKind::FromStart;
                return result;
            }
            if (isRecognizedUnsupportedTimespan(normalized)) {
                throw Mars2GribGenericException(
                    "Recognized but unsupported `timespan`: '" + *value + "'",
                    Here());
            }
            if (!isSupportedTimespanString(normalized)) {
                throw Mars2GribGenericException(
                    "String `timespan` is not in the supported language-defined set: '" + *value + "'",
                    Here());
            }

            const long seconds = parseDurationStringSeconds(normalized, "timespan");
            if (seconds <= 0 || seconds % 3600L != 0) {
                throw Mars2GribGenericException(
                    "String `timespan` is not in the supported language-defined set: '" + *value + "'",
                    Here());
            }

            result.timespanKind = TimespanKind::Duration;
            result.timespanInSeconds = seconds;
            return result;
        }

        if (auto value = get_opt<long>(mars, "timespan")) {
            if (*value < 1) {
                throw Mars2GribGenericException(
                    "Integer `timespan` must be >= 1 hour",
                    Here());
            }
            if (!options.allowNonEnumeratedPositiveIntegerTimespanHours &&
                !isSupportedTimespanHours(*value)) {
                throw Mars2GribGenericException(
                    "Integer `timespan` is not language-enumerated and compatibility option is disabled",
                    Here());
            }

            result.timespanKind = TimespanKind::Duration;
            result.timespanInSeconds = checkedHoursToSeconds(*value, "timespan");
            return result;
        }

        throw Mars2GribGenericException("Unsupported type for `timespan`", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `timespan`",
            Here()));
    }
}


/**
 * @brief Build the parsed typed `stattype` block sequence.
 *
 * @param mars MARS dictionary.
 * @return Parsed ordered block sequence, or an empty sequence when `stattype`
 *         is absent.
 * @throws Mars2GribGenericException on wrong-type, malformed, or unsupported
 *         `stattype` input.
 */
template <class MarsDict_t>
inline ParsedStatTypeBlocks makeStattypeBlocks(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    try {
        if (!has(mars, "stattype")) {
            return ParsedStatTypeBlocks{};
        }
        if (auto value = get_opt<std::string>(mars, "stattype")) {
            return parseStatType(*value);
        }
        throw Mars2GribGenericException("`stattype` must be a string", Here());
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `stattype`",
            Here()));
    }
}


/**
 * @brief Build the optional explicit parameter-side increment.
 *
 * The extracted value must be strictly positive when present.
 *
 * @param par Parameter dictionary.
 * @return Explicit increment in seconds, or `std::nullopt` when absent.
 * @throws Mars2GribGenericException on wrong-type or non-positive input.
 */
template <class ParDict_t>
inline std::optional<long> makeTimeIncrementInSeconds(const ParDict_t& par) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    try {
        if (!has(par, "timeIncrementInSeconds")) {
            return std::nullopt;
        }

        std::optional<long> result;
        if (auto value = get_opt<long>(par, "timeIncrementInSeconds")) {
            result = *value;
        } else if (auto value = get_opt<std::string>(par, "timeIncrementInSeconds")) {
            result = parseLongStrict(*value, "timeIncrementInSeconds");
        } else {
            throw Mars2GribGenericException(
                "Unsupported type for `timeIncrementInSeconds`",
                Here());
        }

        if (*result <= 0) {
            throw Mars2GribGenericException(
                "`timeIncrementInSeconds` must be strictly positive when present",
                Here());
        }

        return result;
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract `timeIncrementInSeconds`",
            Here()));
    }
}


/**
 * @brief Build the complete normalized ProductTimeSpecInputState.
 *
 * This function is the outer extraction orchestrator. It invokes the smaller
 * make-functions in deterministic order, assembles the resulting fragments into
 * one complete intermediate state object, and adds one stable outer extraction
 * context on failure.
 *
 * Extraction order is:
 *
 * 1. option snapshot;
 * 2. mandatory MARS context;
 * 3. direct anchor sources;
 * 4. year/month reference anchor;
 * 5. step;
 * 6. timespan representation;
 * 7. stattype blocks;
 * 8. explicit parameter-side increment.
 *
 * @param innerMostType Caller-supplied innermost statistical-processing type.
 * @param mars MARS dictionary.
 * @param par Parameter dictionary.
 * @param opt Option dictionary.
 * @return Complete intermediate extraction state.
 * @throws Mars2GribGenericException if any extraction fragment fails.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
inline state_detail::ProductTimeSpecInputState makeProductTimeSpecInputState_or_throw(
    tables::TypeOfStatisticalProcessing innerMostType,
    const MarsDict_t& mars,
    const ParDict_t& par,
    const OptDict_t& opt) {
    try {
        state_detail::ProductTimeSpecInputState state;

        state.innerMostTypeOfStatisticalProcessing = innerMostType;
        state.options = makeOptions(opt);

        const auto marsContext = makeMarsContext(mars);
        state.marsClass = marsContext.marsClass;
        state.marsStream = marsContext.marsStream;
        state.marsType = marsContext.marsType;
        state.marsParamId = marsContext.marsParamId;

        state.marsDate = makeMarsDate(mars);
        state.marsTime = makeMarsTime(mars);
        state.marsHdate = makeMarsHdate(mars);

        const auto yearMonth = makeMarsYearMonth(mars);
        state.marsYear = yearMonth.year;
        state.marsMonth = yearMonth.month;

        state.stepInSeconds = makeStepInSeconds(mars);

        const auto timespanInfo = makeTimespanInfo(mars, state.options);
        state.timespanKind = timespanInfo.timespanKind;
        state.timespanInSeconds = timespanInfo.timespanInSeconds;

        state.stattypeBlocks = makeStattypeBlocks(mars);
        state.timeIncrementInSeconds = makeTimeIncrementInSeconds(par);

        return state;
    } catch (...) {
        std::throw_with_nested(Mars2GribGenericException(
            "Failed to extract normalized ProductTimeSpec input",
            Here()));
    }
}

}  // namespace metkit::mars2grib::product_time_spec::input_detail
