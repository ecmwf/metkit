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
 * @file ProductTimeSpecInputOptions.h
 * @brief Option snapshot extraction for ProductTimeSpecInput.
 */

#pragma once

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/product-time-spec/detail/input/ProductTimeSpecInputDictionaryAccess.h"

namespace metkit::mars2grib::product_time_spec::input_detail {

template <class OptDict_t>
inline ProductTimeSpecOptions parseOptions(const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;

    ProductTimeSpecOptions options;

    if (has(opt, "allowDefaultTimeIncrementInSeconds")) {
        options.allowDefaultTimeIncrementInSeconds =
            get_opt<bool>(opt, "allowDefaultTimeIncrementInSeconds").value();
    } else {
        options.allowDefaultTimeIncrementInSeconds =
            metkit::mars2grib::defaults::allowDefaultTimeIncrementInSeconds;
    }

    if (has(opt, "allowZeroLengthFsWindow")) {
        options.allowZeroLengthFsWindow =
            get_opt<bool>(opt, "allowZeroLengthFsWindow").value();
    } else {
        options.allowZeroLengthFsWindow =
            metkit::mars2grib::defaults::allowZeroLengthFsWindow;
    }

    if (has(opt, "allowNonEnumeratedPositiveIntegerTimespanHours")) {
        options.allowNonEnumeratedPositiveIntegerTimespanHours =
            get_opt<bool>(opt, "allowNonEnumeratedPositiveIntegerTimespanHours").value();
    } else {
        options.allowNonEnumeratedPositiveIntegerTimespanHours =
            metkit::mars2grib::defaults::allowNonEnumeratedPositiveIntegerTimespanHours;
    }

    if (has(opt, "allowRedundantTimeIncrement")) {
        options.allowRedundantTimeIncrement =
            get_opt<bool>(opt, "allowRedundantTimeIncrement").value();
    } else {
        options.allowRedundantTimeIncrement =
            metkit::mars2grib::defaults::allowRedundantTimeIncrement;
    }

    if (has(opt, "allowMissingTimespanForInstantProduct")) {
        options.allowMissingTimespanForInstantProduct =
            get_opt<bool>(opt, "allowMissingTimespanForInstantProduct").value();
    } else {
        options.allowMissingTimespanForInstantProduct =
            metkit::mars2grib::defaults::allowMissingTimespanForInstantProduct;
    }

    return options;
}

}  // namespace metkit::mars2grib::product_time_spec::input_detail
