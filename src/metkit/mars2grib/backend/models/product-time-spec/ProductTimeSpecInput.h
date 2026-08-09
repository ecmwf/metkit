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
/// @file ProductTimeSpecInput.h
/// @brief Model-local normalized input aggregate for `build_ProductTimeSpec_or_throw`.
///
/// Exposes `ProductTimeSpecInput` and `make_ProductTimeSpecInput_or_throw`, the
/// Stage-1 backend-model entry point that assembles one normalized temporal
/// input snapshot from deduction outputs plus the model-level boolean policy
/// fields still needed after deduction.
///
/// This header owns:
/// - one model-local aggregate carrying already-resolved deduction outputs;
/// - copying of the flat ProductTimeSpec model-policy booleans from the typed
///   options dictionary;
/// - best-effort diagnostic JSON serialization of that aggregate;
/// - the deduction-driven input assembly step of ProductTimeSpec model
///   construction.
///
/// This header does NOT:
/// - classify ProductTimeSpec anchor, shape, or increment semantics;
/// - construct ProductTimeSpec semantic artifacts;
/// - canonicalize the final ProductTimeSpec model.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "eckit/types/Date.h"
#include "eckit/types/Time.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/class.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/date.h"
#include "metkit/mars2grib/backend/deductions/fcmonth.h"
#include "metkit/mars2grib/backend/deductions/hdate.h"
#include "metkit/mars2grib/backend/deductions/isSynoptic.h"
#include "metkit/mars2grib/backend/deductions/month.h"
#include "metkit/mars2grib/backend/deductions/paramId.h"
#include "metkit/mars2grib/backend/deductions/simulationRegime.h"
#include "metkit/mars2grib/backend/deductions/simulationType.h"
#include "metkit/mars2grib/backend/deductions/stattype.h"
#include "metkit/mars2grib/backend/deductions/step.h"
#include "metkit/mars2grib/backend/deductions/stream.h"
#include "metkit/mars2grib/backend/deductions/time.h"
#include "metkit/mars2grib/backend/deductions/timeIncrement.h"
#include "metkit/mars2grib/backend/deductions/timespan.h"
#include "metkit/mars2grib/backend/deductions/type.h"
#include "metkit/mars2grib/backend/deductions/typeOfTimeIncrement.h"
#include "metkit/mars2grib/backend/deductions/year.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec {


namespace detail {

///
/// @brief Resolve whether the product requires the fake-double-loop single-loop source form.
///
/// The rule is driven by the normalized `(class, stream)` product whitelist.
///
/// @tparam MarsDict_t MARS dictionary type.
/// @tparam ParDict_t Parameter dictionary type.
/// @tparam OptDict_t Options dictionary type.
/// @param[in] mars MARS dictionary.
/// @param[in] par Parameter dictionary.
/// @param[in] opt Options dictionary.
/// @return `true` when the product requires the fake-double-loop source representation.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
///         evaluation failure.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
bool requiresFakeDoubleLoopSingleLoopRepresentation_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                             const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const std::string marsClass  = deductions::resolve_Class_or_throw(mars, par, opt);
        const std::string marsStream = deductions::resolve_Stream_or_throw(mars, par, opt);

        const bool isE6Statistics = marsClass == "e6" && (marsStream == "sttd" || marsStream == "stte");

        const bool isSeas6Statistics = (marsClass == "od" || marsClass == "rd" || marsClass == "c3") &&
                                       (marsStream == "sfmd" || marsStream == "shmd");

        const bool isHistoricalStatistics =
            (marsClass == "gh" || marsClass == "eh") && (marsStream == "msmm" || marsStream == "rfsd");

        return isE6Statistics || isSeas6Statistics || isHistoricalStatistics;
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribModelException(
            "Failed to execute `requiresFakeDoubleLoopSingleLoopRepresentation_or_throw`", Here()));
    }
}

///
/// @brief Resolve whether the product requires a fake second canonical loop.
///
/// The rule is driven by the normalized `(type, paramId)` product whitelist.
///
/// @tparam MarsDict_t MARS dictionary type.
/// @tparam ParDict_t Parameter dictionary type.
/// @tparam OptDict_t Options dictionary type.
/// @param[in] mars MARS dictionary.
/// @param[in] par Parameter dictionary.
/// @param[in] opt Options dictionary.
/// @return `true` when the product requires a fake second canonical loop.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
///         evaluation failure.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
bool requiresFakeSingleLoopDoubleLoopRepresentation_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                             const OptDict_t& opt) {
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        constexpr std::array<std::string_view, 4> indexStatisticsTypes = {{"efi", "efic", "sot", "cpf"}};

        const std::string marsType = deductions::resolve_Type_or_throw(mars, par, opt);
        const long marsParamId     = deductions::resolve_ParamId_or_throw(mars, par, opt);

        const bool typeIsValid    = std::any_of(indexStatisticsTypes.begin(), indexStatisticsTypes.end(),
                                                [&marsType](auto value) { return marsType == value; });
        const bool paramIdIsValid = marsParamId / 1000 == 132;

        return typeIsValid && paramIdIsValid;
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribModelException(
            "Failed to execute `requiresFakeSingleLoopDoubleLoopRepresentation_or_throw`", Here()));
    }
}

/**
 * @brief Verify whether the innermost statistical processing is allowed at step zero.
 *
 * The operation is accepted when:
 * - the processing type is `Missing`, representing the instant case; or
 * - the processing type is `Accumulation`; or
 * - the extended zero-length from-start operation set is enabled and the
 *   processing type is `Average`, `Minimum`, or `Maximum`.
 *
 * @param[in] innerTypeOfStatisticalProcessing Innermost processing type to validate.
 * @param[in] allowExtendedSetOfOperationsForZeroLengthFsWindow Whether the
 *            extended zero-length from-start operation set is enabled.
 * @return `true` when the processing type is allowed at step zero; otherwise `false`.
 * @throws Mars2GribModelException If evaluation unexpectedly fails.
 */
inline bool isAllowed_InnerTypeOfStatisticalProcessingAtStepZero(
    const metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing& innerTypeOfStatisticalProcessing,
    const bool allowExtendedSetOfOperationsForZeroLengthFsWindow) {

    using metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {

        static constexpr std::array<TypeOfStatisticalProcessing, 3> extendedSetOfOperation = {
            {TypeOfStatisticalProcessing::Average, TypeOfStatisticalProcessing::Minimum,
             TypeOfStatisticalProcessing::Maximum}};

        const bool isMissing      = innerTypeOfStatisticalProcessing == TypeOfStatisticalProcessing::Missing;
        const bool isAccumulation = innerTypeOfStatisticalProcessing == TypeOfStatisticalProcessing::Accumulation;
        const bool isInExtendedOperationSet = std::any_of(
            extendedSetOfOperation.begin(), extendedSetOfOperation.end(),
            [&innerTypeOfStatisticalProcessing](auto value) { return innerTypeOfStatisticalProcessing == value; });

        if (isMissing) {
            return true;
        }

        if (isAccumulation) {
            return true;
        }

        if (isInExtendedOperationSet) {
            return allowExtendedSetOfOperationsForZeroLengthFsWindow;
        }

        return false;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `isAllowed_InnerTypeOfStatisticalProcessingAtStepZero`", Here()));
    }
}

}  // namespace detail

struct ProductTimeSpecInput {

    /// @brief Normalized MARS `class` used by representation policies.
    std::string marsClass{};

    /// @brief Normalized MARS `stream` used by representation policies.
    std::string marsStream{};

    /// @brief Normalized MARS `type` used by local temporal consistency rules.
    std::string marsType{};

    /// @brief Normalized MARS `paramId` used by shape-specific policy hooks.
    long marsParamId{-1};

    /// @brief Product whitelist requires the fake-double-loop single-loop source form.
    bool requiresFakeDoubleLoopSingleLoopRepresentation{false};

    /// @brief Product whitelist requires a fake second canonical loop.
    bool requiresFakeSingleLoopDoubleLoopRepresentation{false};

    /// @brief Optional normalized MARS `year` source retained as a raw calendar year.
    std::optional<long> marsYear{};

    /// @brief Optional normalized MARS `month` source retained as a raw calendar month.
    std::optional<long> marsMonth{};

    /// @brief Optional normalized MARS `date` source.
    std::optional<eckit::Date> marsDate{};

    /// @brief Optional normalized MARS `time` source.
    std::optional<eckit::Time> marsTime{};

    /// @brief Optional normalized MARS `hdate` source.
    std::optional<eckit::Date> marsHdate{};

    /// @brief Optional normalized MARS `fcmonth` source retained as a raw forecast month.
    std::optional<long> marsFcmonth{};

    /// @brief Normalized synoptic/non-synoptic product flag.
    bool isSynoptic{false};

    /// @brief Normalized simulation regime used by domain and shape matchers.
    deductions::SimulationRegime regime{deductions::SimulationRegime::IFS};

    /// @brief Normalized simulation type used by domain and shape matchers.
    deductions::SimulationType simulationType{deductions::SimulationType::Forecast};

    /// @brief Optional normalized `step` duration.
    std::optional<deductions::TimeDuration> step{};

    /// @brief Optional normalized `timespan` source representation.
    deductions::Timespan timespan{};

    /// @brief Optional parsed `stattype` block sequence.
    deductions::ParsedStatTypeBlocks stattype{};

    /// @brief Optional normalized explicit time increment.
    std::optional<deductions::TimeDuration> timeIncrement{};

    /// @brief type of time increment
    // tables::TypeOfTimeIntervals typeOfTimeIncrement{tables::TypeOfTimeIntervals::Missing};

    /// @brief Permit policy-defaulted increments for eligible products.
    bool allowDefaultTimeIncrement{false};

    /// @brief Permit the zero-length from-start window case.
    bool allowZeroLengthFsWindow{false};

    /// @brief Permit extended set of operations for zero-length from-start window.
    bool allowExtendedSetOfOperationsForZeroLengthFsWindow{false};

    /// @brief Retained only as diagnostic input context; not used by model logic.
    bool allowNonEnumeratedPositiveIntegerTimespanHours{false};

    /// @brief Permit explicit increments that are semantically redundant.
    bool allowRedundantTimeIncrement{false};

    /// @brief Permit missing `timespan` as the instant-product representation.
    bool allowMissingTimespanForInstantProduct{false};

    /// @brief Permit missing `timespan` as a compatibility form for eligible statistical products.
    bool allowMissingTimespanForStatisticalProduct{false};

    /// @brief Caller-supplied innermost statistical processing type.
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing{
        tables::TypeOfStatisticalProcessing::Missing};

    /// @brief Whether the innermost statistical processing is allowed in the step-zero from-start special case.
    bool isAllowedInnerTypeOfStatisticalProcessingAtStepZero{false};

    ///
    /// @brief Serialize this normalized input snapshot as JSON.
    ///
    /// This function is best-effort and never throws. When serialization fails,
    /// a stable fallback sentence is returned so diagnostics can still attach
    /// context without risking a secondary exception.
    ///
    /// @return One JSON object string on success, or a stable fallback sentence
    ///         if serialization itself fails.
    ///
    std::string to_json() const noexcept {
        try {
            std::ostringstream out;
            out << '{' << detail::jsonQuote_modelInput("marsClass") << ':' << detail::jsonQuote_modelInput(marsClass)
                << ',' << detail::jsonQuote_modelInput("marsStream") << ':' << detail::jsonQuote_modelInput(marsStream)
                << ',' << detail::jsonQuote_modelInput("marsType") << ':' << detail::jsonQuote_modelInput(marsType)
                << ',' << detail::jsonQuote_modelInput("marsParamId") << ':' << marsParamId << ','
                << detail::jsonQuote_modelInput("requiresFakeDoubleLoopSingleLoopRepresentation") << ':'
                << (requiresFakeDoubleLoopSingleLoopRepresentation ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("requiresFakeSingleLoopDoubleLoopRepresentation") << ':'
                << (requiresFakeSingleLoopDoubleLoopRepresentation ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("marsYear") << ':' << detail::optionalLongJson_modelInput(marsYear)
                << ',' << detail::jsonQuote_modelInput("marsMonth") << ':'
                << detail::optionalLongJson_modelInput(marsMonth) << ',' << detail::jsonQuote_modelInput("marsDate")
                << ':' << detail::optionalDateJson_modelInput(marsDate) << ','
                << detail::jsonQuote_modelInput("marsTime") << ':' << detail::optionalTimeJson_modelInput(marsTime)
                << ',' << detail::jsonQuote_modelInput("marsHdate") << ':'
                << detail::optionalDateJson_modelInput(marsHdate) << ',' << detail::jsonQuote_modelInput("marsFcmonth")
                << ':' << detail::optionalLongJson_modelInput(marsFcmonth) << ','
                << detail::jsonQuote_modelInput("isSynoptic") << ':' << (isSynoptic ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("regime") << ':'
                << detail::jsonQuote_modelInput(regime == deductions::SimulationRegime::AIFS ? "AIFS" : "IFS") << ','
                << detail::jsonQuote_modelInput("simulationType") << ':'
                << detail::jsonQuote_modelInput(simulationType == deductions::SimulationType::Analysis ? "Analysis"
                                                                                                       : "Forecast")
                << ',' << detail::jsonQuote_modelInput("step") << ':' << detail::optionalDurationJson_modelInput(step)
                << ',' << detail::jsonQuote_modelInput("timespan") << ':'
                << detail::optionalTimespanJson_modelInput(timespan) << ',' << detail::jsonQuote_modelInput("stattype")
                << ':' << detail::optionalStattypeJson_modelInput(stattype) << ','
                << detail::jsonQuote_modelInput("timeIncrement") << ':'
                << detail::optionalDurationJson_modelInput(timeIncrement) << ','
                << detail::jsonQuote_modelInput("allowDefaultTimeIncrement") << ':'
                << (allowDefaultTimeIncrement ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowZeroLengthFsWindow") << ':'
                << (allowZeroLengthFsWindow ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowExtendedSetOfOperationsForZeroLengthFsWindow") << ':'
                << (allowExtendedSetOfOperationsForZeroLengthFsWindow ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowNonEnumeratedPositiveIntegerTimespanHours") << ':'
                << (allowNonEnumeratedPositiveIntegerTimespanHours ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowRedundantTimeIncrement") << ':'
                << (allowRedundantTimeIncrement ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowMissingTimespanForInstantProduct") << ':'
                << (allowMissingTimespanForInstantProduct ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowMissingTimespanForStatisticalProduct") << ':'
                << (allowMissingTimespanForStatisticalProduct ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("innerMostTypeOfStatisticalProcessing") << ':'
                << detail::jsonQuote_modelInput(
                       tables::enum2name_TypeOfStatisticalProcessing_or_throw(innerMostTypeOfStatisticalProcessing))
                << ',' << detail::jsonQuote_modelInput("isAllowedInnerTypeOfStatisticalProcessingAtStepZero") << ':'
                << (isAllowedInnerTypeOfStatisticalProcessingAtStepZero ? "true" : "false") << '}';
            return out.str();
        }
        catch (...) {
            return std::string{
                "{\"error\":\"ProductTimeSpecInput::to_json() failed while building diagnostic context\"}"};
        }
    }
};

///
/// @brief Build one normalized ProductTimeSpec model input snapshot.
///
/// @section Model-input assembly contract
///   - Reads (MARS): all ProductTimeSpec-related source keys via deductions
///   - Reads (par):  all ProductTimeSpec-related parameter keys via deductions
///   - Reads (opt):  through deductions that require options and directly for
///                   ProductTimeSpec model-policy booleans via typed bool reads
///   - Writes:       none
///   - Side effects: deduction-layer logging only
///   - Failure mode: throws `Mars2GribModelException` (nested-with)
///
/// Stage 1 assembles one model-local input aggregate from the deduction layer
/// and from the subset of typed boolean options that remain relevant after
/// deduction. It performs no ProductTimeSpec semantic classification or final
/// model construction.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] innerMostTypeOfStatisticalProcessing Caller-supplied innermost
///            statistical processing type.
/// @param[in] mars  MARS dictionary.
/// @param[in] par   Parameter dictionary.
/// @param[in] opt   Options dictionary.
///
/// @return Complete normalized ProductTimeSpec input snapshot.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on any
///         deduction, typed-option read, or aggregation failure, with the
///         original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
ProductTimeSpecInput make_ProductTimeSpecInput_or_throw(
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing, const MarsDict_t& mars,
    const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::backend::models::product_time_spec::detail::
        isAllowed_InnerTypeOfStatisticalProcessingAtStepZero;
    using metkit::mars2grib::backend::models::product_time_spec::detail::
        requiresFakeDoubleLoopSingleLoopRepresentation_or_throw;
    using metkit::mars2grib::backend::models::product_time_spec::detail::
        requiresFakeSingleLoopDoubleLoopRepresentation_or_throw;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        ProductTimeSpecInput input;
        input.marsClass   = deductions::resolve_Class_or_throw(mars, par, opt);
        input.marsStream  = deductions::resolve_Stream_or_throw(mars, par, opt);
        input.marsType    = deductions::resolve_Type_or_throw(mars, par, opt);
        input.marsParamId = deductions::resolve_ParamId_or_throw(mars, par, opt);
        input.requiresFakeDoubleLoopSingleLoopRepresentation =
            requiresFakeDoubleLoopSingleLoopRepresentation_or_throw(mars, par, opt);
        input.requiresFakeSingleLoopDoubleLoopRepresentation =
            requiresFakeSingleLoopDoubleLoopRepresentation_or_throw(mars, par, opt);
        input.marsYear    = deductions::resolve_Year_opt(mars, par, opt);
        input.marsMonth   = deductions::resolve_Month_opt(mars, par, opt);
        input.marsDate    = deductions::resolve_Date_opt(mars, par, opt);
        input.marsTime    = deductions::resolve_Time_opt(mars, par, opt);
        input.marsHdate   = deductions::resolve_Hdate_opt(mars, par, opt);
        input.marsFcmonth = deductions::resolve_Fcmonth_opt(mars, par, opt);

        input.step          = deductions::resolve_Step_opt(mars, par, opt);
        input.timespan      = deductions::resolve_Timespan_or_throw(mars, par, opt);
        input.stattype      = deductions::resolve_Stattype_or_throw(mars, par, opt);
        input.timeIncrement = deductions::resolve_TimeIncrement_opt(mars, par, opt);


        input.isSynoptic     = deductions::resolve_IsSynoptic_or_throw(mars, par, opt);
        input.regime         = deductions::resolve_SimulationRegime_or_throw(mars, par, opt);
        input.simulationType = deductions::resolve_SimulationType_or_throw(mars, par, opt);

        input.allowDefaultTimeIncrement = get_or_throw<bool>(opt, "allowDefaultTimeIncrement");
        input.allowZeroLengthFsWindow   = get_or_throw<bool>(opt, "allowZeroLengthFsWindow");
        input.allowExtendedSetOfOperationsForZeroLengthFsWindow =
            get_or_throw<bool>(opt, "allowExtendedSetOfOperationsForZeroLengthFsWindow");
        input.allowNonEnumeratedPositiveIntegerTimespanHours =
            get_or_throw<bool>(opt, "allowNonEnumeratedPositiveIntegerTimespanHours");
        input.allowRedundantTimeIncrement           = get_or_throw<bool>(opt, "allowRedundantTimeIncrement");
        input.allowMissingTimespanForInstantProduct = get_or_throw<bool>(opt, "allowMissingTimespanForInstantProduct");
        input.allowMissingTimespanForStatisticalProduct =
            get_or_throw<bool>(opt, "allowMissingTimespanForStatisticalProduct");
        input.innerMostTypeOfStatisticalProcessing = innerMostTypeOfStatisticalProcessing;
        input.isAllowedInnerTypeOfStatisticalProcessingAtStepZero =
            isAllowed_InnerTypeOfStatisticalProcessingAtStepZero(
                input.innerMostTypeOfStatisticalProcessing, input.allowExtendedSetOfOperationsForZeroLengthFsWindow);


        MARS2GRIB_LOG_RESOLVE(
            [&]() { return std::string{"`ProductTimeSpecInput` built from deductions: "} + input.to_json(); }());

        return input;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException("Failed to build `ProductTimeSpecInput` from deduction outputs", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models::product_time_spec
