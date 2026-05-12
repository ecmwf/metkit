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

#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_NumberOfFrequencies_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        long numberOfFrequencies = get_or_throw<long>(par, "numberOfFrequencies");

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`numberOfFrequencies` resolved from input dictionaries: value=";
            logMsg += std::to_string(numberOfFrequencies);
            return logMsg;
        }());

        return numberOfFrequencies;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `numberOfFrequencies` from input dictionaries", Here()));
    };

    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
