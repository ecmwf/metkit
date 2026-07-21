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
 * @file ProductTimeSpecInputToJson.h
 * @brief Standalone JSON serialization helpers for ProductTimeSpecInput state.
 *
 * This header owns the standalone JSON formatting helpers used to serialize the
 * normalized ProductTimeSpecInput state without requiring a large out-of-class
 * `to_json()` implementation block at the end of the public input header.
 *
 * The helpers in this file format already-normalized values only. They do not
 * perform dictionary access, parsing, validation, or classification.
 *
 * Serialization rules:
 *
 * - absent optionals are emitted as the unquoted literal `null`;
 * - enums are emitted by symbolic name;
 * - dates are emitted as quoted ISO datetimes using `defaultMarsTime`;
 * - times are emitted as quoted zero-padded `HH:MM:SS` strings;
 * - `ParsedStatTypeBlocks` are emitted in stored order.
 *
 * The resulting JSON describes normalized state, not original source spelling
 * or source storage types.
 */

#pragma once

#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::product_time_spec::input_detail {

/**
 * @brief Serializes an optional date as one complete JSON scalar.
 *
 * A present date is combined with `defaultMarsTime` and emitted as a quoted ISO
 * DateTime. Absence is emitted as the unquoted literal `null`.
 *
 * @param value Optional date.
 * @return Complete JSON scalar text.
 * @throws DateTime, formatting, or allocation exceptions may propagate.
 */
inline std::string optionalDateJson_tmp(const std::optional<eckit::Date>& value) {
    if (!value) return "null";
    return jsonQuote(eckit::DateTime(*value, defaultMarsTime).iso(true));
}

/**
 * @brief Serializes an optional time as one complete JSON scalar.
 *
 * A present value is emitted as quoted zero-padded `HH:MM:SS`; absence is
 * emitted as the unquoted literal `null`.
 *
 * @param value Optional time.
 * @return Complete JSON scalar text.
 * @throws Formatting or allocation exceptions may propagate.
 */
inline std::string optionalTimeJson_tmp(const std::optional<eckit::Time>& value) {
    if (!value) return "null";
    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << value->hours() << ':'
        << std::setw(2) << value->minutes() << ':' << std::setw(2) << value->seconds();
    return jsonQuote(out.str());
}

/**
 * @brief Serializes one complete normalized ProductTimeSpecInput state snapshot.
 *
 * The output object includes every normalized source field, parsed stattype
 * block, mandatory context string, explicit increment, caller-supplied
 * innermost processing type, and option value.
 *
 * The function is intentionally state-based rather than object-based so callers
 * can reuse it from thin wrappers such as `ProductTimeSpecInput::to_json()`.
 *
 * @param marsDate Optional normalized `date` source.
 * @param marsTime Optional normalized `time` source.
 * @param marsHdate Optional normalized hindcast date source.
 * @param marsYear Optional normalized reference-anchor year.
 * @param marsMonth Optional normalized reference-anchor month.
 * @param stepInSeconds Optional normalized step.
 * @param timespanKind Normalized source timespan category.
 * @param timespanInSeconds Optional duration payload for `timespanKind == Duration`.
 * @param stattypeBlocks Parsed typed `stattype` blocks in stored order.
 * @param marsClass Mandatory lowercase MARS class.
 * @param marsStream Mandatory lowercase MARS stream.
 * @param marsType Mandatory lowercase MARS type.
 * @param timeIncrementInSeconds Optional explicit parameter-side increment.
 * @param innerMostTypeOfStatisticalProcessing Caller-supplied innermost processing type.
 * @param options Complete normalized option snapshot.
 * @return One complete JSON object string.
 * @throws Formatting, allocation, ISO conversion, jsonQuote(), and enum-name
 *         exceptions may propagate.
 */
inline std::string productTimeToJson(
    const std::optional<eckit::Date>& marsDate,
    const std::optional<eckit::Time>& marsTime,
    const std::optional<eckit::Date>& marsHdate,
    const std::optional<long>& marsYear,
    const std::optional<long>& marsMonth,
    const std::optional<long>& stepInSeconds,
    TimespanKind timespanKind,
    const std::optional<long>& timespanInSeconds,
    const ParsedStatTypeBlocks& stattypeBlocks,
    const std::string& marsClass,
    const std::string& marsStream,
    const std::string& marsType,
    const long& marsParamId,
    const std::optional<long>& timeIncrementInSeconds,
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing,
    const ProductTimeSpecOptions& options) {

    std::ostringstream out;
    out << '{'
        << jsonQuote("marsDate") << ':' << optionalDateJson_tmp(marsDate) << ','
        << jsonQuote("marsTime") << ':' << optionalTimeJson_tmp(marsTime) << ','
        << jsonQuote("marsHdate") << ':' << optionalDateJson_tmp(marsHdate) << ','
        << jsonQuote("marsYear") << ':' << (marsYear ? std::to_string(*marsYear) : "null") << ','
        << jsonQuote("marsMonth") << ':' << (marsMonth ? std::to_string(*marsMonth) : "null") << ','
        << jsonQuote("stepInSeconds") << ':' << (stepInSeconds ? std::to_string(*stepInSeconds) : "null") << ','
        << jsonQuote("timespanKind") << ':' << jsonQuote(name(timespanKind)) << ','
        << jsonQuote("timespanInSeconds") << ':' << (timespanInSeconds ? std::to_string(*timespanInSeconds) : "null") << ','
        << jsonQuote("stattypeBlocks") << ":[";

    for (std::size_t i = 0; i < stattypeBlocks.size(); ++i) {
        if (i != 0) out << ',';
        const auto& block = stattypeBlocks[i];
        out << '{' << jsonQuote("timeRange") << ":{"
            << jsonQuote("unit") << ':' << jsonQuote(name(block.timeRange.unit)) << ','
            << jsonQuote("length") << ':' << block.timeRange.length << "},"
            << jsonQuote("typeOfStatisticalProcessing") << ':'
            << jsonQuote(name(block.typeOfStatisticalProcessing)) << '}';
    }

    out << "],"
        << jsonQuote("marsClass") << ':' << jsonQuote(marsClass) << ','
        << jsonQuote("marsStream") << ':' << jsonQuote(marsStream) << ','
        << jsonQuote("marsType") << ':' << jsonQuote(marsType) << ','
        << jsonQuote("marsParamId") << ':' << marsParamId << ','
        << jsonQuote("timeIncrementInSeconds") << ':'
        << (timeIncrementInSeconds ? std::to_string(*timeIncrementInSeconds) : "null") << ','
        << jsonQuote("innerMostTypeOfStatisticalProcessing") << ':'
        << jsonQuote(name(innerMostTypeOfStatisticalProcessing)) << ','
        << jsonQuote("options") << ":{"
        << jsonQuote("allowDefaultTimeIncrementInSeconds") << ':'
        << (options.allowDefaultTimeIncrementInSeconds ? "true" : "false") << ','
        << jsonQuote("allowZeroLengthFsWindow") << ':'
        << (options.allowZeroLengthFsWindow ? "true" : "false") << ','
        << jsonQuote("allowNonEnumeratedPositiveIntegerTimespanHours") << ':'
        << (options.allowNonEnumeratedPositiveIntegerTimespanHours ? "true" : "false") << ','
        << jsonQuote("allowRedundantTimeIncrement") << ':'
        << (options.allowRedundantTimeIncrement ? "true" : "false") << ','
        << jsonQuote("allowMissingTimespanForInstantProduct") << ':'
        << (options.allowMissingTimespanForInstantProduct ? "true" : "false")
        << '}';

    return out.str();
}

}  // namespace metkit::mars2grib::product_time_spec::input_detail
