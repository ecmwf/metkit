/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "Mars2Grib.h"
#include <iostream>
#include <limits>
#include <utility>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/frontend/encoderConfig.h"

#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"

#include "metkit/mars2grib/backend/SpecializedEncoder.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib {

namespace impl {

std::unique_ptr<metkit::codes::CodesHandle> setValues(const eckit::LocalConfiguration& misc,
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

}  // namespace impl


Mars2Grib::Mars2Grib() : opts_{} {}
Mars2Grib::Mars2Grib(const Options& opts) : opts_{opts} {}
Mars2Grib::Mars2Grib(const eckit::LocalConfiguration& opts) : opts_{impl::readOptions(opts)} {}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc,
                                                              const std::vector<double>& values) {

    // The encoder is fully specialized here in place
    using encoder = metkit::mars2grib::backend::SpecializedEncoder<eckit::LocalConfiguration, eckit::LocalConfiguration,
                                                                   Options, metkit::codes::CodesHandle>;
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::printExtendedStack;

    try {
        // Frontend
        const auto conf = frontend::buildEncoderConfig(mars);

        // Backend
        auto sample = encoder{conf}.encode(mars, misc, opts_);

        // Values
        return impl::setValues(misc, values, std::move(sample));
    }
    catch (const std::exception& e) {
        printExtendedStack(e);
        throw;  // TODO: do not rethrow through the API boundaries
    }
    catch (...) {
        std::cerr << "Unknown exception was caught!" << std::endl;
        throw;  // TODO: do not rethrow through the API boundaries
    }
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc,
                                                              const std::vector<float>& values) {
    // ecCodes does not support setting float values (yet), we have to set them as doubles for now!
    return encode(mars, misc, std::vector<double>{values.begin(), values.end()});
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const std::vector<double>& values) {
    const eckit::LocalConfiguration misc{};
    return encode(mars, misc, values);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const std::vector<float>& values) {
    const eckit::LocalConfiguration misc{};
    return encode(mars, misc, values);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc,
                                                              const double* values, size_t length) {
    return encode(mars, misc, std::vector<double>{values, values + length});
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const eckit::LocalConfiguration& misc,
                                                              const float* values, size_t length) {
    return encode(mars, misc, std::vector<float>{values, values + length});
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const double* values, size_t length) {
    const eckit::LocalConfiguration misc{};
    return encode(mars, misc, values, length);
}

std::unique_ptr<metkit::codes::CodesHandle> Mars2Grib::encode(const eckit::LocalConfiguration& mars,
                                                              const float* values, size_t length) {
    const eckit::LocalConfiguration misc{};
    return encode(mars, misc, values, length);
}

}  // namespace metkit::mars2grib
