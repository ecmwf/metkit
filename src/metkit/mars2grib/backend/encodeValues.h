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
 * @file encodeValues.h
 * @brief Low-level utility for GRIB payload encoding via non-owning memory spans.
 *
 * This header defines `encodeValues`, a **terminal encoding operation** that
 * bridges raw numeric data to the physical GRIB message representation.
 *
 * By utilizing `metkit::codes::Span`, this utility achieves **zero-copy
 * data passing** from the caller to the encoding engine. A temporary copy
 * is only performed if a type conversion (e.g., `float` to `double`) is
 * strictly required by the underlying ecCodes API.
 *
 * The logic is designed to trigger the internal ecCodes encoding machinery,
 * which performs:
 * - Bitmap construction from missing values
 * - Value packing and compression (e.g., CCSDS, Simple Packing)
 * - Message size resolution
 *
 * @ingroup mars2grib_backend
 */
#pragma once

// System includes
#include <algorithm>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

// Project includes
#include "metkit/codes/api/CodesTypes.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend {

template<typename T>
using Span = metkit::codes::Span<T>;

/**
 * @brief Inject numeric field values and resolve data-section bitmasking.
 *
 * A `Values` encoding operation represents the **physical realization of
 * the GRIB Data Section**.
 *
 * Unlike metadata encoding, which is combinatorial, value encoding is
 * **procedural and performance-critical**. The use of `Span<Val_t>` ensures
 * that the payload is passed by reference, avoiding unnecessary allocations
 * on the hot path.
 *
 * The role of this function is to:
 * - Configure the GRIB handle's **Data Representation** state
 * - Bind the numeric payload to the `values` key
 * - Handle **Precision Casting** only when native support is unavailable
 *
 * ------------------------------------------------------------------------
 *
 * @section internal_behavior Internal ecCodes Behavior
 *
 * Upon setting the `values` key, the underlying ecCodes engine performs:
 * - **Missing Value Scanning**: If `bitmapPresent` is enabled, the input
 * buffer is scanned, and a bitmask is constructed using the specified
 * `missingValue`.
 * - **Packing Execution**: The data is compressed according to the
 * `packingType` resolved during the header encoding phase.
 *
 * ------------------------------------------------------------------------
 *
 * @tparam Val_t      Numeric precision of input (must be `float` or `double`)
 * @tparam MiscDict_t Type of the dictionary containing auxiliary metadata
 * @tparam OptDict_t  Type of the dictionary containing auxiliary metadata
 *
 * @param[in]     values  Non-owning span of numeric values (Payload)
 * @param[in]     misc    Dictionary containing bitmap and missing value keys
 * @param[in]     opt     Dictionary containing options
 * @param[in,out] handle  The GRIB handle to be updated
 *
 * @return The updated GRIB handle containing the encoded payload
 */
template <typename Val_t, class MiscDict_t, class OptDict_t, class OutDict_t>
void encodeValues( Span<const Val_t> values,
                   const MiscDict_t& misc,
                   const OptDict_t& opt,
                   OutDict_t& handle) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        static_assert(std::is_floating_point_v<Val_t>, "encodeValues: Val_t must be float or double");

        // 1. Configure Bitmap and Metadata State
        const bool bitmapPresent = get_opt<bool>(misc, "bitmapPresent").value_or(false);
        set_or_throw(handle, "bitmapPresent", bitmapPresent);

        if (bitmapPresent) {
            // Resolve missing value sentinel cast to double for ecCodes compatibility
            const double missingValue = get_opt<double>(misc, "missingValue")
                                            .value_or(static_cast<double>(std::numeric_limits<Val_t>::max()));
            set_or_throw(handle, "missingValue", missingValue);
        }

        // 2. Physical Value Injection
        // Current ecCodes implementation requires double-precision for the 'values' key.
        // If input is float, we perform an explicit deep-copy cast to double.
        if constexpr (std::is_same_v<Val_t, double>) {
            set_or_throw( handle, "values", values );
        }
        else {
            // Deep copy-cast from float to double for legacy API support
            std::vector<double> dValues;
            dValues.reserve(values.size());

            const Val_t* raw = values.data();
            for (std::size_t i = 0; i < values.size(); ++i) {
                dValues.push_back(static_cast<double>(raw[i]));
            }

            set_or_throw(handle, "values", Span<const double>{dValues});
        }

    }
    catch ( ... ) {
        std::throw_with_nested(Mars2GribGenericException(
            "Critical failure in SpecializedEncoder execution",
            Here())
        );
    }

}

}  // namespace metkit::mars2grib::backend