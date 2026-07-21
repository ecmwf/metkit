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

#include <optional>
#include <sstream>
#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/class.h"
#include "metkit/mars2grib/backend/deductions/dateTime.h"
#include "metkit/mars2grib/backend/deductions/hindcastDateTime.h"
#include "metkit/mars2grib/backend/deductions/paramId.h"
#include "metkit/mars2grib/backend/deductions/stattype.h"
#include "metkit/mars2grib/backend/deductions/step.h"
#include "metkit/mars2grib/backend/deductions/stream.h"
#include "metkit/mars2grib/backend/deductions/timeIncrement.h"
#include "metkit/mars2grib/backend/deductions/timespan.h"
#include "metkit/mars2grib/backend/deductions/type.h"
#include "metkit/mars2grib/backend/deductions/yearMonthDateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {


struct ProductTimeSpecInput {

    /// @brief Normalized MARS `class` used by representation policies.
    std::string marsClass{};

    /// @brief Normalized MARS `stream` used by representation policies.
    std::string marsStream{};

    /// @brief Normalized MARS `type` used by local temporal consistency rules.
    std::string marsType{};

    /// @brief Normalized MARS `paramId` used by shape-specific policy hooks.
    long marsParamId{-1};

    /// @brief Optional direct initial-conditions datetime source.
    std::optional<eckit::DateTime> dateTime{};

    /// @brief Optional direct hindcast label datetime source.
    std::optional<eckit::DateTime> hindcastDateTime{};

    /// @brief Optional direct year/month reference-anchor datetime source.
    std::optional<eckit::DateTime> yearMonthDateTime{};

    /// @brief Optional normalized `step` duration.
    std::optional<deductions::TimeDuration> step{};

    /// @brief Optional normalized `timespan` source representation.
    std::optional<deductions::Timespan> timespan{};

    /// @brief Optional parsed `stattype` block sequence.
    std::optional<deductions::ParsedStatTypeBlocks> stattype{};

    /// @brief Optional normalized explicit time increment.
    std::optional<deductions::TimeDuration> timeIncrement{};

    /// @brief Permit policy-defaulted increments for eligible products.
    bool allowDefaultTimeIncrementInSeconds{false};

    /// @brief Permit the zero-length from-start window case.
    bool allowZeroLengthFsWindow{false};

    /// @brief Retained only as diagnostic input context; not used by model logic.
    bool allowNonEnumeratedPositiveIntegerTimespanHours{false};

    /// @brief Permit explicit increments that are semantically redundant.
    bool allowRedundantTimeIncrement{false};

    /// @brief Permit missing `timespan` as the instant-product representation.
    bool allowMissingTimespanForInstantProduct{false};

    /// @brief Caller-supplied innermost statistical processing type.
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing{
        tables::TypeOfStatisticalProcessing::Missing};

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
            out << '{' << detail::jsonQuote_modelInput("marsClass") << ':' << detail::jsonQuote_modelInput(marsClass) << ','
                << detail::jsonQuote_modelInput("marsStream") << ':' << detail::jsonQuote_modelInput(marsStream) << ','
                << detail::jsonQuote_modelInput("marsType") << ':' << detail::jsonQuote_modelInput(marsType) << ','
                << detail::jsonQuote_modelInput("marsParamId") << ':' << marsParamId << ','
                << detail::jsonQuote_modelInput("dateTime") << ':' << detail::optionalDateTimeJson_modelInput(dateTime) << ','
                << detail::jsonQuote_modelInput("hindcastDateTime") << ':'
                << detail::optionalDateTimeJson_modelInput(hindcastDateTime) << ','
                << detail::jsonQuote_modelInput("yearMonthDateTime") << ':'
                << detail::optionalDateTimeJson_modelInput(yearMonthDateTime) << ','
                << detail::jsonQuote_modelInput("step") << ':' << detail::optionalDurationJson_modelInput(step) << ','
                << detail::jsonQuote_modelInput("timespan") << ':' << detail::optionalTimespanJson_modelInput(timespan) << ','
                << detail::jsonQuote_modelInput("stattype") << ':' << detail::optionalStattypeJson_modelInput(stattype) << ','
                << detail::jsonQuote_modelInput("timeIncrement") << ':'
                << detail::optionalDurationJson_modelInput(timeIncrement) << ','
                << detail::jsonQuote_modelInput("allowDefaultTimeIncrementInSeconds") << ':'
                << (allowDefaultTimeIncrementInSeconds ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowZeroLengthFsWindow") << ':'
                << (allowZeroLengthFsWindow ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowNonEnumeratedPositiveIntegerTimespanHours") << ':'
                << (allowNonEnumeratedPositiveIntegerTimespanHours ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowRedundantTimeIncrement") << ':'
                << (allowRedundantTimeIncrement ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("allowMissingTimespanForInstantProduct") << ':'
                << (allowMissingTimespanForInstantProduct ? "true" : "false") << ','
                << detail::jsonQuote_modelInput("innerMostTypeOfStatisticalProcessing") << ':'
                << detail::jsonQuote_modelInput(
                       tables::enum2name_TypeOfStatisticalProcessing_or_throw(innerMostTypeOfStatisticalProcessing))
                << '}';
            return out.str();
        } catch (...) {
            return std::string{"{\"error\":\"ProductTimeSpecInput::to_json() failed while building diagnostic context\"}"};
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
    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing,
    const MarsDict_t& mars,
    const ParDict_t& par,
    const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        ProductTimeSpecInput input;
        input.marsClass = deductions::resolve_Class_or_throw(mars, par, opt);
        input.marsStream = deductions::resolve_Stream_or_throw(mars, par, opt);
        input.marsType = deductions::resolve_Type_or_throw(mars, par, opt);
        input.marsParamId = deductions::resolve_ParamId_or_throw(mars, par, opt);

        input.dateTime = deductions::resolve_DateTime_opt(mars, par, opt);
        input.hindcastDateTime = deductions::resolve_HindcastDateTime_opt(mars, par, opt);
        input.yearMonthDateTime = deductions::resolve_YearMonthDateTime_opt(mars, par, opt);
        input.step = deductions::resolve_Step_opt(mars, par, opt);
        input.timespan = deductions::resolve_Timespan_opt(mars, par, opt);
        input.stattype = deductions::resolve_Stattype_opt(mars, par, opt);
        input.timeIncrement = deductions::resolve_TimeIncrement_opt(mars, par, opt);
        input.allowDefaultTimeIncrementInSeconds =
            get_or_throw<bool>(opt, "allowDefaultTimeIncrementInSeconds");
        input.allowZeroLengthFsWindow =
            get_or_throw<bool>(opt, "allowZeroLengthFsWindow");
        input.allowNonEnumeratedPositiveIntegerTimespanHours =
            get_or_throw<bool>(opt, "allowNonEnumeratedPositiveIntegerTimespanHours");
        input.allowRedundantTimeIncrement =
            get_or_throw<bool>(opt, "allowRedundantTimeIncrement");
        input.allowMissingTimespanForInstantProduct =
            get_or_throw<bool>(opt, "allowMissingTimespanForInstantProduct");
        input.innerMostTypeOfStatisticalProcessing = innerMostTypeOfStatisticalProcessing;

        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`ProductTimeSpecInput` built from deductions: "} + input.to_json();
        }());

        return input;
    } catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to build `ProductTimeSpecInput` from deduction outputs", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::models
