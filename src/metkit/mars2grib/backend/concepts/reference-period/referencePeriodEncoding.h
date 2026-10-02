/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file referencePeriodEncoding.h
/// @brief Implementation of the GRIB `referencePeriod` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **referencePeriod concept** within the mars2grib backend.
///
/// The concept encodes the reference-period block of PDT 4.106/4.107/4.112/
/// 4.131-4.135: the additional parameters (SOT percentiles), the model-climate
/// time ranges and sample size, and for EFI/SOT the reference dataset and
/// relation.
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `referencePeriodApplicable`
/// - Runtime execution via `ReferencePeriodOp`
/// - Strict error handling with contextual concept exceptions
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of `concepts`
/// to avoid ambiguity and potential conflicts with the C++20 `concept` language
/// feature and related standard headers.
///
/// This is a deliberate design choice and must not be changed.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <array>
#include <string>
#include <vector>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/reference-period/referencePeriodEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/numberOfAdditionalParametersForReferencePeriod.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `referencePeriod` concept.
///
/// The concept is applicable in the allocate and preset stages of the Product Definition Section.
///
template <std::size_t Stage, std::size_t Section, ReferencePeriodType Variant>
constexpr bool referencePeriodApplicable() {
    return (Section == SecProductDefinitionSection) && (Stage == StageAllocate || Stage == StagePreset);
}

namespace detail {

/// @brief One additional parameter of the reference period, as a GRIB2 scaled value.
struct ReferencePeriodParameter {
    long scaleFactor;
    long scaledValue;
};

///
/// @brief Split the MARS `quantile` of a SOT product into its two percentiles.
///
/// The MARS form is `<low>-<high>:<denominator>`, with denominator 100 * 10^k (e.g. `90-99:100`, `1-10:100`,
/// `900-995:1000`). The percentile nearer to the median comes first (SOT90 = {90, 99}, SOT10 = {10, 1}, the order
/// ecCodes uses to build `mars.quantile`, ECC-2000), and each value uses the smallest exact scale factor.
///
inline std::array<ReferencePeriodParameter, 2> sotPercentiles_or_throw(const std::string& quantile) {

    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    const auto dash  = quantile.find('-');
    const auto colon = quantile.find(':');
    if (dash == std::string::npos || colon == std::string::npos || colon < dash) {
        throw Mars2GribGenericException("Invalid SOT quantile `" + quantile + "`: expected `<low>-<high>:<den>`",
                                        Here());
    }

    const long low         = std::stol(quantile.substr(0, dash));
    const long high        = std::stol(quantile.substr(dash + 1, colon - dash - 1));
    const long denominator = std::stol(quantile.substr(colon + 1));

    long scale    = 0;
    long expected = 100;
    while (expected < denominator) {
        expected *= 10;
        ++scale;
    }
    if (expected != denominator || !(0 < low && low < high && high < denominator)) {
        throw Mars2GribGenericException(
            "Invalid SOT quantile `" + quantile + "`: expected 0 < low < high < denominator = 100 * 10^k", Here());
    }

    const auto scaled = [scale](long value) {
        long factor = scale;
        while (factor > 0 && value % 10 == 0) {
            value /= 10;
            --factor;
        }
        return ReferencePeriodParameter{factor, value};
    };

    if (2 * low >= denominator) {
        return {scaled(low), scaled(high)};
    }
    return {scaled(high), scaled(low)};
}

}  // namespace detail

///
/// @brief Execute the `referencePeriod` concept operation.
///
/// StageAllocate:
/// - `numberOfAdditionalParametersForReferencePeriod` (2 for SOT, else from the parameter dictionary or 0)
/// - for SOT, the two percentiles as additional parameters. They are set here rather than in StagePreset because
///   ecCodes builds `mars.quantile` from them as soon as the local section (`marsStream`) is set, which happens
///   before the preset of the Product Definition Section.
/// - the reference-period time ranges of the model climate, when the parameter dictionary provides them
///   (`numberOfReforecastYearsInModelClimate` in years, `numberOfDaysInClimateSamplingWindow` in days)
///
/// StagePreset:
/// - EFI/SOT (`efi`, `efic`, `sot`): `typeOfReferenceDataset` = 2 (reforecast), `typeOfRelationToReferenceDataset`
///   = 20 (EFI) or 21 (SOT). For the other products the relation follows from the paramId (ecCodes).
/// - `sampleSizeOfReferencePeriod` from `sampleSizeOfModelClimate`, when given
///
/// TODO: the start of the reference period (`yearOfStartOfReferencePeriod`, ...) is not encoded yet: it is not part
///       of the input metadata.
///
template <std::size_t Stage, std::size_t Section, ReferencePeriodType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ReferencePeriodOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if constexpr (referencePeriodApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(referencePeriod);

            const auto marsType = get_or_throw<std::string>(mars, "type");

            // =============================================================
            // StageAllocate
            // =============================================================
            if constexpr (Stage == StageAllocate) {

                const long numAdditionalParametersForReferencePeriod =
                    deductions::resolve_numberOfAdditionalParametersForReferencePeriod_or_throw(mars, par, opt);
                set_or_throw<long>(out, "numberOfAdditionalParametersForReferencePeriod",
                                   numAdditionalParametersForReferencePeriod);

                if (marsType == "sot") {
                    const auto quantile    = get_or_throw<std::string>(mars, "quantile");
                    const auto percentiles = detail::sotPercentiles_or_throw(quantile);

                    // GRIB1 local definition 19 carries the outer percentile as `efiOrder`
                    const auto efiOrder = get_opt<long>(par, "efiOrder");
                    if (efiOrder && percentiles[1].scaleFactor == 0 && *efiOrder != percentiles[1].scaledValue) {
                        throw Mars2GribGenericException("SOT `efiOrder` (" + std::to_string(*efiOrder) +
                                                            ") does not match MARS `quantile` (" + quantile + ")",
                                                        Here());
                    }

                    set_or_throw<std::vector<long>>(out, "scaleFactorOfAdditionalParameterForReferencePeriod",
                                                    {percentiles[0].scaleFactor, percentiles[1].scaleFactor});
                    set_or_throw<std::vector<long>>(out, "scaledValueOfAdditionalParameterForReferencePeriod",
                                                    {percentiles[0].scaledValue, percentiles[1].scaledValue});
                }

                // Model climate: N reforecast years, sampled in a window of D days (code table 4.4: 4 = year, 2 = day)
                const auto years = get_opt<long>(par, "numberOfReforecastYearsInModelClimate");
                const auto days  = get_opt<long>(par, "numberOfDaysInClimateSamplingWindow");
                if (years) {
                    std::vector<long> units{4};
                    std::vector<long> lengths{*years};
                    if (days) {
                        units.push_back(2);
                        lengths.push_back(*days);
                    }
                    set_or_throw<long>(out, "numberOfReferencePeriodTimeRanges", static_cast<long>(units.size()));
                    set_or_throw<std::vector<long>>(out, "indicatorOfUnitForTimeRangeForReferencePeriod", units);
                    set_or_throw<std::vector<long>>(out, "lengthOfTimeRangeForReferencePeriod", lengths);
                }
            }

            // =============================================================
            // StagePreset
            // =============================================================
            if constexpr (Stage == StagePreset) {

                if (marsType == "efi" || marsType == "efic" || marsType == "sot") {
                    // Code table 4.100: 2 = Reforecast (Hindcast)
                    set_or_throw<long>(out, "typeOfReferenceDataset", 2);
                    // Code table 4.101: 20 = Extreme Forecast Index (EFI), 21 = Shift of Tails (SOT)
                    set_or_throw<long>(out, "typeOfRelationToReferenceDataset", marsType == "sot" ? 21 : 20);
                }

                if (const auto sampleSize = get_opt<long>(par, "sampleSizeOfModelClimate")) {
                    set_or_throw<long>(out, "sampleSizeOfReferencePeriod", *sampleSize);
                }
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(referencePeriod, "Unable to set `referencePeriod` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(referencePeriod, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
