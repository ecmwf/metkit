#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractNumber(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has(keyword)) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `" + keyword + "` required to extract MARS keyword `" + keyword + "`", Here());
        }

        const long value = grib.getLong(keyword);
        set_or_throw<long>(mars, keyword, value);

        if (grib.has("numberOfForecastsInEnsemble")) {
            const long numberOfForecastsInEnsemble = grib.getLong("numberOfForecastsInEnsemble");

            if (numberOfForecastsInEnsemble != 0) {
                misc.set("numberOfForecastsInEnsemble", numberOfForecastsInEnsemble);

                if (grib.getString("class") != "ai") {
                    if (grib.has("typeOfEnsembleForecast")) {
                        const long typeOfEnsembleForecast = grib.getLong("typeOfEnsembleForecast");
                        misc.set("typeOfEnsembleForecast", typeOfEnsembleForecast);
                    }
                    else if (grib.has("eps")) {
                        const long typeOfEnsembleForecast = grib.getLong("eps");
                        misc.set("typeOfEnsembleForecast", typeOfEnsembleForecast);
                    }
                }
            }
        }
        else if (grib.getString("type") != "me") {
            throw Grib2MarsGenericException(
                "Missing GRIB key `numberOfForecastsInEnsemble` required to extract MARS keyword `" + keyword + "`", Here());
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl