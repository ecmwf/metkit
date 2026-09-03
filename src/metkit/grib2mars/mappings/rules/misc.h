#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict, class OptDict_t>
void extractMisc(const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc, const OptDict_t& opts) {
    using metkit::grib2mars::utils::dict_traits::get_or_throw;
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        (void)opts;
        if (grib.has("generatingProcessIdentifier")) {
            const auto generatingProcessIdentifier = grib.getLong("generatingProcessIdentifier");
            misc.set("generatingProcessIdentifier", generatingProcessIdentifier);
        }

        const auto type = get_or_throw<std::string>(mars, "type");

        if (type != "ai" && type != "fc" && grib.has("typeOfProcessedData")) {
            const auto typeOfProcessedData = grib.getString("typeOfProcessedData");
            if (typeOfProcessedData != "missing") {
                misc.set("typeOfProcessedData", typeOfProcessedData);
            }
        }

        if (type == "eme" || type == "me") {
            if (grib.has("numberOfComponents")) {
                const auto numberOfComponents = grib.getLong("numberOfComponents");
                misc.set("numberOfComponents", numberOfComponents);
            }

            if (grib.has("modelErrorType")) {
                const auto modelErrorType = grib.getLong("modelErrorType");
                misc.set("modelErrorType", modelErrorType);
            }
        }

        if (type == "es" || type == "em" || type == "ses") {
            if (!grib.has("numberOfForecastsInEnsemble")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `numberOfForecastsInEnsemble` required for derived ensemble forecast", Here());
            }
            const long numberOfForecastsInEnsemble = grib.getLong("numberOfForecastsInEnsemble");
            misc.set("numberOfForecastsInEnsemble", numberOfForecastsInEnsemble);
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MISC keywords", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl