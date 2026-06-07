/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file all.h
/// @brief Conversion rules used by the grib2mars mapper.
#pragma once

#include "metkit/grib2mars/mappings/Grib2MarsReturnValue.h"
#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

// Codes wrapper types
#include "metkit/codes/api/CodesAPI.h"

namespace metkit::grib2mars::rules {

/// @brief Apply all registered conversion rules and return a result object.
template <class OutDict_t>
Grib2MarsResult<OutDict_t> convertAll(const metkit::codes::CodesHandle& grib) {

    using metkit::grib2mars::utils::dict_traits::clone_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        std::unique_ptr<OutDict_t> out                  = std::make_unique<OutDict_t>();
        std::unique_ptr<eckit::LocalConfiguration> misc = std::make_unique<eckit::LocalConfiguration>();

        return Grib2MarsResult<OutDict_t>{std::move(*out), std::move(*misc)};
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(Grib2MarsGenericException("Failed to convertAll grib message", Here()));
    }
}

}  // namespace metkit::grib2mars::rules
