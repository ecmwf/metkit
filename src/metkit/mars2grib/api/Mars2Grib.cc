/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file Mars2Grib.cc
 * @brief Implementation of the Mars2Grib public encoding API.
 *
 * This file contains the implementation of the `Mars2Grib` class declared
 * in `Mars2Grib.h`.
 *
 * It provides the concrete orchestration logic that:
 * - builds the internal encoder configuration from the MARS dictionary
 * - invokes the specialized backend encoder
 * - injects field values into the resulting GRIB handle
 *
 * This file intentionally contains **no GRIB semantics** and **no deduction
 * logic**. All domain-specific decisions are delegated to lower layers.
 *
 * ---
 *
 * ## Error propagation
 *
 * The current implementation propagates all exceptions across the API
 * boundary after printing extended diagnostic information.
 *
 * This behavior is intentional for early integration phases and may be
 * revised in the future to provide API-stable error wrapping.
 *
 * ---
 *
 * ## Scope
 *
 * - This file is part of the **Mars2Grib public API implementation**
 * - It is not intended for direct use by end users
 * - Its behavior defines the observable semantics of `Mars2Grib::encode`
 *
 * @ingroup mars2grib_api
 */


#include "Mars2Grib.h"

// System includes
#include <iostream>
#include <limits>
#include <regex>
#include <utility>

// eckit
#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"

// ecCodes API
#include "metkit/codes/api/CodesAPI.h"

// mars2grib frontend
#include "metkit/mars2grib/frontend/encoderConfig.h"

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// backend encoder
#include "metkit/mars2grib/backend/SpecializedEncoder.h"

// error handling
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib {

namespace impl {

/**
 * @brief Inject field values into an existing GRIB handle.
 *
 * This helper function applies value-related metadata and sets the
 * actual field values on a pre-initialized GRIB handle.
 *
 * The following optional keys may be read from the `misc` dictionary:
 * - `bitmapPresent` (bool)
 * - `missingValue` (double)
 *
 * If `bitmapPresent` is enabled, the missing value is explicitly set
 * on the GRIB handle before assigning the values array.
 *
 * @param[in] misc
 *   Auxiliary metadata dictionary.
 *
 * @param[in] values
 *   Field values to encode.
 *
 * @param[in,out] handle
 *   GRIB handle produced by the backend encoder.
 *
 * @return
 *   The same GRIB handle, with values injected.
 *
 * @throws eckit::NotImplemented
 *   If unsupported value scaling is requested.
 */
std::unique_ptr<metkit::codes::CodesHandle> encodeValues(const eckit::LocalConfiguration& misc,
                                                         const std::vector<double>& values,
                                                         std::unique_ptr<metkit::codes::CodesHandle> handle) {

    using metkit::mars2grib::utils::dict_traits::get_opt;

    auto bitmapPresent = get_opt<bool>(misc, "bitmapPresent").value_or(false);
    auto missingValue  = get_opt<double>(misc, "missingValue").value_or(std::numeric_limits<double>::max());

    handle->set("bitmapPresent", bitmapPresent);
    if (bitmapPresent) {
        handle->set("missingValue", missingValue);
    }

    if (get_opt<long>(misc, "values-scale-factor").value_or(1.0) != 1.0) {
        throw eckit::NotImplemented{"Handling scale factor is not implemented!", Here()};
    }

    handle->set("values", values);

    return handle;
};

/**
 * @brief Read Mars2Grib options from a configuration object.
 *
 * This helper function maps a subset of keys from an
 * `eckit::LocalConfiguration` into a strongly typed `Options` object.
 *
 * Only explicitly present keys are applied; all others retain their
 * default values.
 *
 * @param[in] conf
 *   Configuration object containing encoder options.
 *
 * @return
 *   A fully initialized `Options` structure.
 */
Options readOptions(const eckit::LocalConfiguration& conf) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    Options opts;
    if (has<bool>(conf, "applyChecks")) {
        opts.applyChecks = get_or_throw<bool>(conf, "applyChecks");
    }
    if (has<bool>(conf, "enableOverride")) {
        opts.enableOverride = get_or_throw<bool>(conf, "enableOverride");
    }
    if (has<bool>(conf, "enableBitsPerValueCompression")) {
        opts.enableBitsPerValueCompression = get_or_throw<bool>(conf, "enableBitsPerValueCompression");
    }
    return opts;
}

/**
 * @brief Fix key `grid` in `mars` if it contains a known incorrect grid
 *
 * This function only exists to support grids which are not supported by eckit::geo, but have been supported by this
 * encoder in the past.
 *
 * @param[in] mars
 *   MARS dictionary, potentially containing a wrong `grid` value
 *
 * @return
 *   The corrected MARS dictionary
 */
eckit::LocalConfiguration fixMarsGrid(eckit::LocalConfiguration mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;

    if (auto marsGrid = get_opt<std::string>(mars, "grid"); marsGrid.has_value()) {
        static const std::regex pattern{R"(L(\d+)x(\d+))"};
        std::smatch match;
        if (std::regex_match(*marsGrid, match, pattern)) {
            const long ni         = std::stol(match[1].str());
            const long nj         = std::stol(match[2].str());
            const double deltaLon = 360.0 / static_cast<double>(ni);
            const double deltaLat = 180.0 / static_cast<double>(nj - 1);
            marsGrid              = std::to_string(deltaLon) + "/" + std::to_string(deltaLat);
            set_or_throw(mars, "grid", *marsGrid);
        }
    }
    return mars;
}

}  // namespace impl


// -----------------------------------------------------------------------------
// Mars2Grib construction
// -----------------------------------------------------------------------------

Mars2Grib::Mars2Grib() : opts_{} {}

Mars2Grib::Mars2Grib(const Options& opts) : opts_{opts} {}

Mars2Grib::Mars2Grib(const eckit::LocalConfiguration& opts) : opts_{impl::readOptions(opts)} {}


// -----------------------------------------------------------------------------
// Encoding interface
// -----------------------------------------------------------------------------


std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<double>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    // Fix a potentially incorrect MARS grid keyword
    const auto fixedMars = impl::fixMarsGrid(mars);

    /**
     * The encoder is fully specialized at this point:
     * - dictionary types are fixed
     * - options type is fixed
     * - output handle type is fixed
     */
    using encoder = metkit::mars2grib::backend::SpecializedEncoder<eckit::LocalConfiguration, eckit::LocalConfiguration,
                                                                   Options, metkit::codes::CodesHandle>;
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::printExtendedStack;

    try {
        // Frontend: derive encoder configuration from MARS dictionary
        const auto headerLayout = frontend::buildEncoderConfig(fixedMars);

        // Backend: construct GRIB header
        auto gribHeader = encoder{headerLayout}.encode(fixedMars, misc, opts_);

        // Inject values and return final GRIB handle
        return impl::encodeValues(misc, values, std::move(gribHeader));
    }
    catch (const std::exception& e) {
        // Print extended diagnostic stack before propagating
        printExtendedStack(e);
        throw;
    }
    catch (...) {
        LOG_DEBUG_LIB(LibMetkit) << "Unknown exception was caught!" << std::endl;
        throw;
    }
}


// -----------------------------------------------------------------------------
// Type adaptation overloads
// -----------------------------------------------------------------------------

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    /**
     * ecCodes currently does not support setting float values directly.
     * Values are therefore promoted to double.
     */
    return encode(std::vector<double>{values.begin(), values.end()}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<double>& values,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(values, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const std::vector<float>& values,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(values, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode(std::vector<double>{values, values + length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc) {
    return encode(std::vector<float>{values, values + length}, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const double* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(values, length, mars, misc);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const float* values, size_t length,
                                                              const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return encode(values, length, mars, misc);
}

}  // namespace metkit::mars2grib
