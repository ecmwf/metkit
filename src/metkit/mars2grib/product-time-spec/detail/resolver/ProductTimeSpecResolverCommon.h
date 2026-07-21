/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include <array>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/utils/generalUtils.h"


namespace metkit::mars2grib::product_time_spec {
namespace resolver_detail {

/**
 * @brief Shared resolver helpers used by multiple specialized stage headers.
 *
 * These helpers must remain available before the detail headers are included,
 * because the split anchor/shape/increment resolvers all depend on the same
 * diagnostic and stage-independent utility functions.
 */

/**
 * @brief Serialize a valid classification triple for diagnostics.
 *
 * The JSON text is embedded in `Mars2GribProductTimeSpecException` so failures
 * can report which valid classification state had already been reached.
 *
 * @param value Valid classification triple.
 * @return Compact JSON object text.
 * @throws Standard-library formatting and allocation failures may propagate.
 */
inline std::string classificationJson(const ProductTimeSpecClassification& value) {
    std::ostringstream out;
    out << '{'
        << jsonQuote("anchorType") << ':' << jsonQuote(name(value.anchorType)) << ','
        << jsonQuote("shapeType") << ':' << jsonQuote(name(value.shapeType)) << ','
        << jsonQuote("incrementType") << ':' << jsonQuote(name(value.incrementType))
        << '}';
    return out.str();
}

/**
 * @brief Raise one stage-tagged ProductTimeSpec resolver failure.
 *
 * This helper centralizes the stable diagnostic payload shared by all resolver
 * stages:
 *
 * - the stage tag;
 * - a human-readable reason;
 * - the serialized normalized input snapshot;
 * - optional classification state;
 * - optional construction-artifact JSON;
 * - optional final-spec JSON.
 *
 * @param stage Resolver stage at which the failure occurred.
 * @param reason Human-readable error message.
 * @param input Normalized input snapshot.
 * @param classification Optional already-resolved classification triple.
 * @param artifactJson Optional serialized construction artifact.
 * @param finalSpecJson Optional serialized final ProductTimeSpec.
 * @throws Mars2GribProductTimeSpecException unconditionally.
 */
template <class Input_t>
[[noreturn]] void fail(ProductTimeSpecStage stage,
                       const std::string& reason,
                       const Input_t& input,
                       const std::optional<ProductTimeSpecClassification>& classification = std::nullopt,
                       const std::string& artifactJson = {},
                       const std::string& finalSpecJson = {}) {
    throw Mars2GribProductTimeSpecException(
        stage,
        reason,
        input.to_json(),
        classification ? classificationJson(*classification) : std::string{},
        artifactJson,
        finalSpecJson,
        Here());
}

/**
 * @brief Return the resolved forecast step in seconds.
 *
 * The missing-step fallback to zero is valid only after the surrounding logic
 * has already established that the current branch permits a missing step, which
 * in the supported domain means `type="an"`.
 *
 * @param input Normalized input snapshot.
 * @return Explicit step in seconds, or zero when absent.
 * @throws Nothing.
 */
template <class Input_t>
long resolvedStep(const Input_t& input) {
    return input.stepInSeconds().value_or(0L);
}

/**
 * @brief Test whether a statistical-processing code is the missing sentinel.
 *
 * @param value Candidate statistical-processing code.
 * @return `true` when the code is `Missing`.
 * @throws Nothing.
 */
inline bool isMissing(tables::TypeOfStatisticalProcessing value) {
    return value == tables::TypeOfStatisticalProcessing::Missing;
}

/**
 * @brief Test whether a time-increment type code is the missing sentinel.
 *
 * @param value Candidate type-of-time-increment code.
 * @return `true` when the code is `Missing`.
 * @throws Nothing.
 */
inline bool isMissing(TypeOfTimeIncrement value) {
    return value == TypeOfTimeIncrement::Missing;
}

}  // namespace resolver_detail

}  // namespace metkit::mars2grib::product_time_spec
