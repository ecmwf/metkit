#pragma once

#include <string>
#include <unordered_map>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

namespace detail {

inline std::string convertPackingType(const std::string& packingType) {
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        static const std::unordered_map<std::string, std::string> packingMap = {
            {"grid_simple", "simple"}, {"grid_complex", "complex"}, {"spectral_complex", "complex"},
            {"grid_ccsds", "ccsds"},   {"grid_ieee", "ccsds"},      {"grid_second_order", "ccsds"},
        };

        const std::unordered_map<std::string, std::string>::const_iterator it = packingMap.find(packingType);

        if (it == packingMap.end()) {
            throw Grib2MarsGenericException("Unhandled packingType `" + packingType + "`", Here());
        }

        const std::string value = it->second;

        return value;
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException("Failed to convert GRIB packingType `" + packingType + "`", Here()));
    }
}

}  // namespace detail

template <class MarsDict, class MiscDict>
void extractPacking(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                    MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has("packingType")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `packingType` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const std::string packingType = grib.getString("packingType");
        const std::string packing     = detail::convertPackingType(packingType);

        set_or_throw<std::string>(mars, keyword, packing);

        if (grib.has("setPackingType")) {
            const std::string setPackingType  = grib.getString("setPackingType");
            const std::string overridePacking = detail::convertPackingType(setPackingType);

            set_or_throw<std::string>(mars, keyword, overridePacking);
        }

        if (grib.has("bitmapPresent")) {
            const long bitmapPresent = grib.getLong("bitmapPresent");

            misc.set("bitmapPresent", bitmapPresent);
        }

        const double missingValue = 9999.0;
        misc.set("missingValue", missingValue);

        long bitsPerValue = 24;

        if (grib.has("bitsPerValue")) {
            bitsPerValue = grib.getLong("bitsPerValue");
        }

        misc.set("bitsPerValue", bitsPerValue);
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl