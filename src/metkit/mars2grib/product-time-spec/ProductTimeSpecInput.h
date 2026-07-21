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
 * @file ProductTimeSpecInput.h
 * @brief Typed extraction and normalization boundary for ProductTimeSpec.
 *
 * This header reads all temporal information required by the ProductTimeSpec
 * resolver from three independent dictionary-like inputs:
 *
 * - the MARS dictionary;
 * - the parameter dictionary;
 * - the option dictionary.
 *
 * ProductTimeSpecInput converts heterogeneous dictionary values into one owned,
 * normalized, read-only snapshot. Downstream classification and construction
 * code must use this snapshot and must not reinterpret the original dictionaries.
 *
 * Extraction owns:
 *
 * - dictionary access through dictionary_access_traits;
 * - lexical parsing;
 * - case normalization;
 * - source-type normalization;
 * - validation of individual values and option dependencies;
 * - preservation of source absence through std::optional;
 * - diagnostic JSON serialization.
 *
 * Extraction does not own:
 *
 * - time-anchor classification;
 * - product-shape classification;
 * - time-increment classification;
 * - cross-classification consistency checks;
 * - construction of ProductTimeSpecAnchor, ProductTimeSpecShape,
 *   ProductTimeSpecIncrement, or ProductTimeSpec.
 *
 * All extraction failures are reported as Mars2GribGenericException. The
 * ProductTimeSpecInput constructor adds a stable outer extraction-context
 * exception and retains the precise original failure as a nested exception.
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
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputStateAndMakers.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputToJson.h"

namespace metkit::mars2grib::product_time_spec {

/**
 * @brief Owned immutable normalized input for ProductTimeSpec resolution.
 *
 * The class reads every relevant source key once and exposes typed const
 * accessors. It retains no reference to any input dictionary.
 *
 * Normalized content includes:
 *
 * - optional label date and time;
 * - optional hindcast date;
 * - optional year/month reference anchor;
 * - optional step in seconds;
 * - timespan category and optional duration;
 * - ordered parsed `stattype` blocks;
 * - mandatory lowercase `class`, `stream`, and `type`;
 * - optional strictly positive explicit increment;
 * - caller-supplied innermost statistical operation;
 * - complete validated option snapshot.
 *
 * Classification and canonical ProductTimeSpec construction are deliberately
 * deferred.
 *
 * @tparam MarsDict_t MARS dictionary type with dictionary_access_traits.
 * @tparam ParDict_t  Parameter dictionary type with dictionary_access_traits.
 * @tparam OptDict_t  Option dictionary type with dictionary_access_traits.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
class ProductTimeSpecInput {
public:
    using InputState = input_detail::state_detail::ProductTimeSpecInputState;

    /**
     * @brief Extracts and normalizes all source values required by the resolver.
     *
     * This public constructor is the extraction entry point. It delegates all
     * parsing, normalization, and validation work to
     * `input_detail::makeProductTimeSpecInputState_or_throw(...)`, which builds
     * one complete intermediate state object before the final object is
     * materialized.
     *
     * Extraction order is deterministic:
     *
     * 1. read and validate option policy;
     * 2. read mandatory MARS context;
     * 3. parse direct and alternative anchor sources;
     * 4. parse step;
     * 5. parse timespan representation;
     * 6. parse `stattype`;
     * 7. read parameter-side increment.
     *
     * Source absence remains explicit. In particular, default time inheritance
     * is not performed by this constructor; it belongs to anchor construction.
     *
     * @param innerMostType Caller-supplied processing type for the innermost real
     *                      statistical window.
     * @param mars          MARS dictionary.
     * @param par           Parameter dictionary.
     * @param opt           Option dictionary.
     *
     * @throws Mars2GribGenericException for malformed, unsupported, incomplete,
     *         contradictory, or incorrectly typed extraction input. The outer
     *         exception states that normalized input extraction failed and the
     *         precise cause is retained as a nested exception.
     *
     * @post Mandatory context strings are present and lowercase.
     * @post `year` and `month` are both present or both absent.
     * @post A present step is non-negative and zero or whole-hour aligned.
     * @post A duration timespan has a strictly positive duration.
     * @post A present explicit increment is strictly positive.
     */
    ProductTimeSpecInput(tables::TypeOfStatisticalProcessing innerMostType,
                         const MarsDict_t& mars,
                         const ParDict_t& par,
                         const OptDict_t& opt) :
        ProductTimeSpecInput(
            input_detail::makeProductTimeSpecInputState_or_throw(
                innerMostType,
                mars,
                par,
                opt)) {}

    /**
     * @brief Returns the optional validated `date` source. Absence means no direct label date was supplied; no inheritance is applied here.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional date.
     * @throws Nothing.
     */
    const std::optional<eckit::Date>& marsDate() const noexcept { return marsDate_; }
    /**
     * @brief Returns the optional validated `time` source. Absence is preserved for later defaulting; a present time without date is a later cross-key classification error.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional time.
     * @throws Nothing.
     */
    const std::optional<eckit::Time>& marsTime() const noexcept { return marsTime_; }
    /**
     * @brief Returns the optional validated hindcast/reforecast date. No `htime` source is consumed by this model.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional hindcast date.
     * @throws Nothing.
     */
    const std::optional<eckit::Date>& marsHdate() const noexcept { return marsHdate_; }
    /**
     * @brief Returns the optional year component of the reference anchor. Its engagement always matches marsMonth().
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional year.
     * @throws Nothing.
     */
    const std::optional<long>& marsYear() const noexcept { return marsYear_; }
    /**
     * @brief Returns the optional month component of the reference anchor. Its engagement always matches marsYear(), and present values are in `[1,12]`.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional month.
     * @throws Nothing.
     */
    const std::optional<long>& marsMonth() const noexcept { return marsMonth_; }
    /**
     * @brief Returns the optional normalized forecast step. Present values are non-negative and either zero or divisible by 3600.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional step in seconds.
     * @throws Nothing.
     */
    const std::optional<long>& stepInSeconds() const noexcept { return stepInSeconds_; }
    /**
     * @brief Returns the normalized source representation category: Missing, Duration, None, or FromStart.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return the TimespanKind value.
     * @throws Nothing.
     */
    TimespanKind timespanKind() const noexcept { return timespanKind_; }
    /**
     * @brief Returns the optional positive duration payload. It is engaged only for a Duration timespan.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional duration in seconds.
     * @throws Nothing.
     */
    const std::optional<long>& timespanInSeconds() const noexcept { return timespanInSeconds_; }
    /**
     * @brief Returns the ordered typed expansion of `stattype`. An empty sequence means no source `stattype` was supplied.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the owned block sequence.
     * @throws Nothing.
     */
    const ParsedStatTypeBlocks& stattypeBlocks() const noexcept { return stattypeBlocks_; }
    /**
     * @brief Returns the mandatory lowercase MARS class used by AIFS and representation policy.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the owned class string.
     * @throws Nothing.
     */
    const std::string& marsClass() const noexcept { return marsClass_; }
    /**
     * @brief Returns the mandatory lowercase MARS stream used with marsClass() by fake-double-loop policy.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the owned stream string.
     * @throws Nothing.
     */
    const std::string& marsStream() const noexcept { return marsStream_; }
    /**
     * @brief Returns the mandatory lowercase MARS type used by rules such as analysis-step consistency.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the owned type string.
     * @throws Nothing.
     */
    const std::string& marsType() const noexcept { return marsType_; }
    /**
     * @brief Returns the mandatory lowercase MARS parameter ID used by rules such as parameter consistency.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the owned parameter ID string.
     * @throws Nothing.
     */
    long marsParamId() const noexcept { return marsParamId_; }
    /**
     * @brief Returns the optional explicit parameter-side increment. Present values are strictly positive, but later policy decides whether they are used or redundant.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the optional increment.
     * @throws Nothing.
     */
    const std::optional<long>& timeIncrementInSeconds() const noexcept { return timeIncrementInSeconds_; }
    /**
     * @brief Returns the caller-supplied innermost statistical-processing type.
     *
     * The value is preserved unchanged during extraction. Compatibility with
     * the classified shape and parsed outer blocks is checked later.
     *
     * @return The stored TypeOfStatisticalProcessing.
     * @throws Nothing.
     */
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing() const noexcept {
        return innerMostTypeOfStatisticalProcessing_;
    }
    /**
     * @brief Returns the complete normalized and validated option snapshot used by all later resolver stages.
     *
     * This accessor exposes owned normalized state and performs no additional
     * parsing, defaulting, classification, or validation.
     *
     * @return const reference to the option snapshot.
     * @throws Nothing.
     */
    const ProductTimeSpecOptions& options() const noexcept { return options_; }

    /**
     * @brief Serializes the complete normalized input snapshot as JSON.
     *
     * The diagnostic object includes every normalized source field, parsed
     * stattype block, mandatory context string, explicit increment, caller
     * processing type, and option value.
     *
     * Absent optionals are emitted as JSON `null`; enums are emitted by symbolic
     * name. The JSON describes normalized state, not original source spelling or
     * source storage type.
     *
     * @return One complete JSON object string.
     * @throws Formatting, allocation, ISO conversion, jsonQuote(), and enum-name
     *         exceptions may propagate.
     */
    std::string to_json() const {
        return input_detail::productTimeToJson(
            marsDate_,
            marsTime_,
            marsHdate_,
            marsYear_,
            marsMonth_,
            stepInSeconds_,
            timespanKind_,
            timespanInSeconds_,
            stattypeBlocks_,
            marsClass_,
            marsStream_,
            marsType_,
            marsParamId_,
            timeIncrementInSeconds_,
            innerMostTypeOfStatisticalProcessing_,
            options_);
    }

private:


    /**
     * @brief Materializes ProductTimeSpecInput from an already-built state object.
     *
     * This constructor performs no parsing, dictionary access, or semantic
     * validation. Its only responsibility is to move the fully normalized
     * intermediate extraction state into the final stored members.
     *
     * It is private because callers are expected to enter through the public
     * extraction constructor, which enforces the full extraction pipeline.
     *
     * @param state Fully normalized intermediate extraction state.
     * @throws Nothing beyond move/copy operations of the stored member types.
     */
    explicit ProductTimeSpecInput(InputState state) :
        marsDate_(std::move(state.marsDate)),
        marsTime_(std::move(state.marsTime)),
        marsHdate_(std::move(state.marsHdate)),
        marsYear_(std::move(state.marsYear)),
        marsMonth_(std::move(state.marsMonth)),
        stepInSeconds_(std::move(state.stepInSeconds)),
        timespanKind_(state.timespanKind),
        timespanInSeconds_(std::move(state.timespanInSeconds)),
        stattypeBlocks_(std::move(state.stattypeBlocks)),
        marsClass_(std::move(state.marsClass)),
        marsStream_(std::move(state.marsStream)),
        marsType_(std::move(state.marsType)),
        marsParamId_(std::move(state.marsParamId)),
        timeIncrementInSeconds_(std::move(state.timeIncrementInSeconds)),
        innerMostTypeOfStatisticalProcessing_(state.innerMostTypeOfStatisticalProcessing),
        options_(std::move(state.options)) {}

    // Direct label-date and label-time sources remain separate so that source
    // absence and the `time`-without-`date` cross-key state remain observable.
    const std::optional<eckit::Date> marsDate_{};
    const std::optional<eckit::Time> marsTime_{};
    // Optional hindcast/reforecast date source.
    const std::optional<eckit::Date> marsHdate_{};
    // Optional year/month reference-anchor pair. Constructor validation
    // guarantees equal engagement and a valid first day of the month.
    const std::optional<long> marsYear_{};
    const std::optional<long> marsMonth_{};
    // Optional step normalized to seconds.
    const std::optional<long> stepInSeconds_{};
    // Timespan representation category and its optional duration payload.
    const TimespanKind timespanKind_{TimespanKind::Missing};
    const std::optional<long> timespanInSeconds_{};
    // Ordered typed expansion of the compact source `stattype`.
    const ParsedStatTypeBlocks stattypeBlocks_{};
    // Mandatory lowercase MARS product context.
    const std::string marsClass_{};
    const std::string marsStream_{};
    const std::string marsType_{};
    const long marsParamId_{-1};  // parameter ID from the mars dictionary.
    // Optional strictly positive explicit increment from the parameter source.
    const std::optional<long> timeIncrementInSeconds_{};
    // Caller-supplied innermost processing type, preserved unchanged.
    const tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing_;
    // Complete validated policy snapshot.
    const ProductTimeSpecOptions options_{};
};

}  // namespace metkit::mars2grib::product_time_spec
