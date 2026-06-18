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

/// @file ecc-1806.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/mappings/rules/common.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl {


/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t>
inline void convertECC1806(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        const auto param = get_or_throw<long>(in, "param");

        // See:
        // https://github.com/ecmwf/eccodes/blob/develop/definitions/grib1/localConcepts/ecmf/paramIdForConversion.def

        switch (param) {
            case 55:
                set_or_throw<long>(out, "param", 228004);
                set_or_throw<std::string>(out, "timespan", "24h");
                break;
            case 56:
                set_or_throw<long>(out, "param", 235168);
                set_or_throw<std::string>(out, "timespan", "24h");
                break;
            case 130232:
                set_or_throw<long>(out, "param", 235135);
                // TODO: Set timespan
                break;
            case 151163:
                set_or_throw<long>(out, "param", 262104);
                break;
            case 151145:
                set_or_throw<long>(out, "param", 262124);
                break;
            case 172146:
                set_or_throw<long>(out, "param", 235033);
                // TODO: Set timespan
                break;
            case 172147:
                set_or_throw<long>(out, "param", 235034);
                // TODO: Set timespan
                break;
            case 172169:
                set_or_throw<long>(out, "param", 235035);
                // TODO: Set timespan
                break;
            case 172175:
                set_or_throw<long>(out, "param", 235036);
                // TODO: Set timespan
                break;
            case 172176:
                set_or_throw<long>(out, "param", 235037);
                // TODO: Set timespan
                break;
            case 172177:
                set_or_throw<long>(out, "param", 235038);
                // TODO: Set timespan
                break;
            case 172178:
                set_or_throw<long>(out, "param", 235039);
                // TODO: Set timespan
                break;
            case 172179:
                set_or_throw<long>(out, "param", 235040);
                // TODO: Set timespan
                break;
            case 174098:
                set_or_throw<long>(out, "param", 262000);
                break;
            case 151175:
                set_or_throw<long>(out, "param", 262118);
                break;
            case 151132:
                set_or_throw<long>(out, "param", 262139);
                break;
            case 151131:
                set_or_throw<long>(out, "param", 262140);
                break;
            case 72:
                set_or_throw<long>(out, "param", 260087);
                break;
            case 73:
                set_or_throw<long>(out, "param", 260097);
                break;
            case 172050:
                set_or_throw<long>(out, "param", 235026);
                // TODO: Set timespan
                break;
            case 172145:
                set_or_throw<long>(out, "param", 235032);
                // TODO: Set timespan
                break;
            case 172189:
                set_or_throw<long>(out, "param", 235189);
                // TODO: Set timespan
                break;
            case 172195:
                set_or_throw<long>(out, "param", 235045);
                // TODO: Set timespan
                break;
            case 172196:
                set_or_throw<long>(out, "param", 235046);
                // TODO: Set timespan
                break;
            case 172197:
                set_or_throw<long>(out, "param", 235047);
                // TODO: Set timespan
                break;

            default:
                break;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to convert input dictionary in convertECC1806", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl
