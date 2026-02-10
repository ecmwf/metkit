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
 * @file CoreOperations.h
 * @brief High-level service layer for GRIB encoding and resolution orchestration.
 *
 * This header defines the `CoreOperations` suite, providing the primary
 * functional building blocks for the mars2grib library.
 *
 * These operations facilitate a staged translation pipeline:
 * 1. **Sanitization**: Normalizing input dictionaries against the language definition.
 * 2. **Header Resolution**: Determining the GRIB structural layout and encoding metadata.
 * 3. **Value Injection**: Physical realization of the GRIB data section.
 * 4. **Diagnostic Capture**: Generating regression data for structural validation.
 *
 * @ingroup mars2grib_core
 */
#pragma once

// System includes
#include <tuple>
#include <string>
#include <ostream>
#include <utility>

// Project includes
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/frontend/make_HeaderLayout.h"
#include "metkit/mars2grib/frontend/header/EncodingPlan.h"
#include "metkit/mars2grib/frontend/header/SpecializedEncoder.h"
#include "metkit/mars2grib/frontend/normalization/normalization.h"
#include "metkit/mars2grib/backend/encodeValues.h"

namespace metkit::mars2grib {


/**
 * @brief Internal engine providing atomic encoding and diagnostic services.
 */
struct CoreOperations {

    /**
     * @brief Normalize input dictionaries against the library language definition.
     *
     * This operation performs key-value sanitization for both MARS and Parameter
     * metadata. It utilizes a **reference-redirection strategy**: if no
     * modification is required, the returned references point to the original
     * inputs; otherwise, they point to the provided scratch buffers.
     *
     * @tparam MarsDict_t MARS dictionary type
     * @tparam ParDict_t  Parameter dictionary type
     * @tparam OptDict_t  Encoding options dictionary type
     *
     * @param[in]  inputMars   Original MARS request
     * @param[in]  inputMisc   Original Parameter metadata
     * @param[in]  opt         Encoding options
     * @param[in]  lang        Language definition (eckit::Value)
     * @param[out] scratchMars Scratch buffer for MARS sanitization
     * @param[out] scratchMisc Scratch buffer for Parameter sanitization
     *
     * @return A tuple containing const references to the active (sanitized) data
     */
    template <class MarsDict_t, class ParDict_t, class OptDict_t>
    static std::tuple<const MarsDict_t&, const ParDict_t&> normalize_if_enabled(
        const MarsDict_t& inputMars,
        const ParDict_t& inputMisc,
        const OptDict_t& opt,
        const eckit::Value& lang,
        MarsDict_t& scratchMars,
        ParDict_t& scratchMisc) {

        const MarsDict_t& activeMars = frontend::normalization::normalize_MarsDict_if_enabled(inputMars, opt, lang, scratchMars);
        const ParDict_t& activePar  = frontend::normalization::normalize_MiscDict_if_enabled(inputMisc, opt, lang, scratchMisc);

        return {activeMars, activePar};
    }

    /**
     * @brief Resolve and encode GRIB header metadata.
     *
     * Executes the structural resolution phase to determine the GRIB layout
     * and triggers the specialized metadata encoder to populate the header
     * sections of the output object.
     *
     * @tparam MarsDict_t MARS dictionary type
     * @tparam ParDict_t  Parameter dictionary type
     * @tparam OptDict_t  Encoding options dictionary type
     * @tparam OutDict_t  Output GRIB handle/dictionary type
     */
    template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
    static std::unique_ptr<OutDict_t> encodeHeader(
        const MarsDict_t& mars,
        const ParDict_t& misc,
        const OptDict_t& opt ) {

        using  metkit::mars2grib::frontend::make_HeaderLayout_or_throw;
        using  metkit::mars2grib::frontend::header::SpecializedEncoder;

        auto layout = make_HeaderLayout_or_throw(mars, opt);

        return SpecializedEncoder<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>{std::move(layout)}.encode(mars, misc, opt);

    }

    /**
     * @brief Inject numeric field values into a GRIB handle.
     *
     * A procedural operation that handles bitmap generation and physical
     * data compression. Utilizes spans for zero-copy data passing.
     *
     * @tparam Val_t      Numeric precision (float or double)
     * @tparam OptDict_t  Encoding options dictionary type
     * @tparam OutDict_t  Output GRIB handle/dictionary type
     */
    template <typename Val_t, class MiscDict_t, class OptDict_t, class OutDict_t>
    static std::unique_ptr<OutDict_t> encodeValues(
        backend::Span<const Val_t> values,
        const MiscDict_t& misc,
        const OptDict_t& opt,
        std::unique_ptr<OutDict_t> handle) {

        metkit::mars2grib::backend::encodeValues( values, misc, opt, *handle);

        return handle;
    }

    /**
     * @brief Capture a structural test point for regression analysis.
     *
     * Serializes the current resolution state (GRIB Blueprint)
     * into a JSON format suitable for external validation tools.
     *
     * @tparam MarsDict_t MARS dictionary type
     * @tparam OptDict_t  Encoding options dictionary type
     */
    template <class MarsDict_t, class OptDict_t>
    static std::string dumpHeaderTest(
        const MarsDict_t& mars,
        const OptDict_t& opt) {

        using metkit::mars2grib::frontend::make_HeaderLayout_or_throw;
        using metkit::mars2grib::frontend::debug::debug_convert_GribHeaderLayoutData_to_json;

        auto layout = make_HeaderLayout_or_throw(mars, opt);

        return debug_convert_GribHeaderLayoutData_to_json(layout);
    }
};

} // namespace metkit::mars2grib