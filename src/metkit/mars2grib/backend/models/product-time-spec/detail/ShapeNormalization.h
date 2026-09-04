/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file ShapeNormalization.h
/// @brief Normalize raw ProductTimeSpec windows to encoder-facing units.
///
/// This internal header owns the raw-to-normalized shape transformation used
/// after raw callback checks succeed. The normalization is intentionally local
/// and explicit:
/// - `timeRange` becomes hours;
/// - `timeIncrement` becomes seconds or missing;
/// - month-based `timeRange` values are converted using the exact placed
///   interval derived from the resolved domain.
///
/// The domain artifact is not modified. It is used only to recover the real
/// placement needed for month-based hour derivation and the outermost window
/// span check.
///
/// @ingroup mars2grib_product_time_spec_detail
///

#pragma once

#include <array>

#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/TimeIncrement.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec::detail {

/// @brief Normalize one raw canonical shape to encoder-facing window units.
///
/// Normalization rules:
/// - `timeRange` becomes whole hours;
/// - `timeIncrement` becomes whole seconds or missing;
/// - month-based `timeRange` values are converted using the exact placed
///   interval starting at the real domain start;
/// - the outermost normalized `timeRange` must equal the real domain span.
///
/// @param[in] input Fully normalized ProductTimeSpec input snapshot.
/// @param[in] domain Resolved raw ProductTimeSpec domain.
/// @param[in] rawShape Resolved raw ProductTimeSpec windows.
/// @return Normalized ProductTimeSpec windows.
/// @throws Mars2GribModelException If normalization or validation fails.
inline shape::ProductTimeSpecShape normalizeShape_or_throw(const ProductTimeSpecInput& input,
                                                           const domain::ProductTimeSpecDomain& domain,
                                                           const shape::ProductTimeSpecShape& rawShape) {
    using metkit::mars2grib::backend::deductions::TimeDuration;
    using metkit::mars2grib::backend::models::product_time_spec::detail::missingIncrement;
    using metkit::mars2grib::backend::tables::TimeUnit;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;
    using metkit::mars2grib::utils::time_arithmetic::addDuration;
    using metkit::mars2grib::utils::time_arithmetic::defaultMarsTime;
    using metkit::mars2grib::utils::time_arithmetic::durationBetween;
    using metkit::mars2grib::utils::time_arithmetic::makeDateTime;

    try {
        if (rawShape.values.empty()) {
            throw Mars2GribModelException("Cannot normalize an empty ProductTimeSpec shape", input.to_json(), Here());
        }

        const auto convertTriviallyConvertibleDurationToSeconds = [&](const TimeDuration& duration, const char* name) {
            try {
                long long seconds = 0;

                switch (duration.unit) {
                    case TimeUnit::Second:
                        return duration.length;
                    case TimeUnit::Minute:
                        seconds = static_cast<long long>(duration.length) * 60LL;
                        break;
                    case TimeUnit::Hour:
                        seconds = static_cast<long long>(duration.length) * 3600LL;
                        break;
                    case TimeUnit::Hours3:
                        seconds = static_cast<long long>(duration.length) * 10800LL;
                        break;
                    case TimeUnit::Hours6:
                        seconds = static_cast<long long>(duration.length) * 21600LL;
                        break;
                    case TimeUnit::Hours12:
                        seconds = static_cast<long long>(duration.length) * 43200LL;
                        break;
                    case TimeUnit::Day:
                        seconds = static_cast<long long>(duration.length) * 86400LL;
                        break;
                    case TimeUnit::Month:
                        throw Mars2GribModelException(
                            std::string{"Cannot trivially convert month-valued `"} + name + "` to seconds",
                            input.to_json(), Here());
                    case TimeUnit::Year:
                    case TimeUnit::Decade:
                    case TimeUnit::Normal:
                    case TimeUnit::Century:
                        throw Mars2GribModelException(
                            std::string{"Cannot trivially convert calendar-valued `"} + name + "` to seconds",
                            input.to_json(), Here());
                    case TimeUnit::Missing:
                        throw Mars2GribModelException(std::string{"Cannot normalize missing `"} + name + "`",
                                                      input.to_json(), Here());
                    default:
                        throw Mars2GribModelException(std::string{"Unsupported time unit in `"} + name + "`",
                                                      input.to_json(), Here());
                }

                if (seconds > static_cast<long long>(std::numeric_limits<long>::max()) ||
                    seconds < static_cast<long long>(std::numeric_limits<long>::min())) {
                    throw Mars2GribModelException(std::string{"Normalized `"} + name + "` in seconds is out of range",
                                                  input.to_json(), Here());
                }

                return static_cast<long>(seconds);
            }
            catch (...) {
                std::throw_with_nested(Mars2GribModelException(
                    std::string{"Failed to normalize `"} + name + "` to seconds", input.to_json(), Here()));
            }
        };

        const auto normalizeTimeRangeToHours = [&](const TimeDuration& rawTimeRange) {
            try {
                if (rawTimeRange.unit == TimeUnit::Month) {
                    const eckit::DateTime normalizationStart =
                        domain.isSynoptic ? makeDateTime(domain.domainStartDateTime.date(), defaultMarsTime())
                                          : domain.domainStartDateTime;
                    const eckit::DateTime normalizationEnd = addDuration(normalizationStart, rawTimeRange);
                    const TimeDuration placedDuration      = durationBetween(normalizationStart, normalizationEnd);

                    if (placedDuration.unit != TimeUnit::Second) {
                        throw Mars2GribModelException(
                            "Placed month-based ProductTimeSpec range did not resolve to seconds", input.to_json(),
                            Here());
                    }

                    if (placedDuration.length % 3600 != 0) {
                        throw Mars2GribModelException(
                            "Month-based ProductTimeSpec range does not convert to whole hours", input.to_json(),
                            Here());
                    }

                    return TimeDuration{placedDuration.length / 3600, TimeUnit::Hour};
                }

                const long seconds = convertTriviallyConvertibleDurationToSeconds(rawTimeRange, "timeRange");

                if (seconds % 3600 != 0) {
                    throw Mars2GribModelException("Sub-hour ProductTimeSpec timeRange values are not supported",
                                                  input.to_json(), Here());
                }

                return TimeDuration{seconds / 3600, TimeUnit::Hour};
            }
            catch (...) {
                std::throw_with_nested(Mars2GribModelException("Failed to normalize ProductTimeSpec timeRange to hours",
                                                               input.to_json(), Here()));
            }
        };

        const auto normalizeTimeIncrementToSecondsOrMissing = [&](const TimeDuration& rawTimeIncrement) {
            try {
                if (rawTimeIncrement.unit == TimeUnit::Missing) {
                    return missingIncrement();
                }

                if (rawTimeIncrement.unit == TimeUnit::Month) {
                    throw Mars2GribModelException("Month-valued ProductTimeSpec timeIncrement is not supported",
                                                  input.to_json(), Here());
                }

                const long seconds = convertTriviallyConvertibleDurationToSeconds(rawTimeIncrement, "timeIncrement");
                return TimeDuration{seconds, TimeUnit::Second};
            }
            catch (...) {
                std::throw_with_nested(
                    Mars2GribModelException("Failed to normalize ProductTimeSpec timeIncrement to seconds or missing",
                                            input.to_json(), Here()));
            }
        };

        constexpr std::array<long, 12> allowedSubmonthlyHours{
            {1L, 3L, 6L, 12L, 18L, 24L, 48L, 72L, 120L, 168L, 240L, 360L}};
        constexpr std::array<long, 4> allowedMonthlyHours{{672L, 696L, 720L, 744L}};

        shape::ProductTimeSpecShape normalizedShape;
        normalizedShape.values.reserve(rawShape.values.size());

        for (std::size_t i = 0; i < rawShape.values.size(); ++i) {
            const shape::ProductTimeSpecWindow& rawWindow = rawShape.values[i];

            const TimeDuration normalizedTimeRange = normalizeTimeRangeToHours(rawWindow.timeRange);
            const TimeDuration normalizedTimeIncrement =
                normalizeTimeIncrementToSecondsOrMissing(rawWindow.timeIncrement);

            const bool rawTimeRangeIsMonthly = rawWindow.timeRange.unit == TimeUnit::Month;

            if (!input.allowNonEnumeratedPositiveIntegerTimespanHours && normalizedTimeRange.length > 0) {

                const bool allowedHourValue =
                    rawTimeRangeIsMonthly ? std::find(allowedMonthlyHours.begin(), allowedMonthlyHours.end(),
                                                      normalizedTimeRange.length) != allowedMonthlyHours.end()
                                          : std::find(allowedSubmonthlyHours.begin(), allowedSubmonthlyHours.end(),
                                                      normalizedTimeRange.length) != allowedSubmonthlyHours.end();

                if (!allowedHourValue) {
                    throw Mars2GribModelException(
                        rawTimeRangeIsMonthly
                            ? "Normalized month-based ProductTimeSpec timeRange is not in the allowed monthly hour "
                              "set: " +
                                  std::to_string(normalizedTimeRange.length)
                            : "Normalized sub-monthly ProductTimeSpec timeRange is not in the allowed hour set: " +
                                  std::to_string(normalizedTimeRange.length),
                        input.to_json(), Here());
                }
            }

            normalizedShape.values.push_back(
                shape::ProductTimeSpecWindow{rawWindow.typeOfStatisticalProcessing, rawWindow.typeOfTimeIncrement,
                                             normalizedTimeRange, normalizedTimeIncrement});
        }

        const eckit::DateTime realDomainStart = domain.isSynoptic
                                                    ? makeDateTime(domain.domainStartDateTime.date(), defaultMarsTime())
                                                    : domain.domainStartDateTime;
        const TimeDuration realDomainSpan     = durationBetween(realDomainStart, domain.domainEndDateTime);

        if (realDomainSpan.unit != TimeUnit::Second) {
            throw Mars2GribModelException("Real ProductTimeSpec domain span did not resolve to seconds",
                                          input.to_json(), Here());
        }

        if (realDomainSpan.length % 3600 != 0) {
            throw Mars2GribModelException("Real ProductTimeSpec domain span does not convert to whole hours",
                                          input.to_json(), Here());
        }

        const long realDomainSpanInHours                    = realDomainSpan.length / 3600;
        const shape::ProductTimeSpecWindow& outermostWindow = normalizedShape.values.front();

        if (outermostWindow.timeRange.unit != TimeUnit::Hour ||
            outermostWindow.timeRange.length != realDomainSpanInHours) {
            throw Mars2GribModelException(
                "Normalized outermost ProductTimeSpec timeRange does not match the real domain span", input.to_json(),
                Here());
        }

        return normalizedShape;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to normalize the ProductTimeSpec shape", input.to_json(), Here()));
    }
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec::detail
