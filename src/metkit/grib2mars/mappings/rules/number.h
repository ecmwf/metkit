#pragma once

#include <string>

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict>
void extractNumber(const std::string& keyword,
                   const metkit::codes::CodesHandle& grib,
                   MarsDict& mars,
                   MiscDict& misc) {
    using metkit::grib2mars::utils::dict_traits::set_or_throw;
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        if (!grib.has("type")) {
            throw Grib2MarsGenericException(
                "Missing GRIB key `type` required to extract MARS keyword `" +
                    keyword + "`",
                Here());
        }

        const std::string type = grib.getString("type");

        if (type == "es" || type == "em") {
            if (!grib.has("numberOfForecastsInEnsemble")) {
                throw Grib2MarsGenericException(
                    "Missing GRIB key `numberOfForecastsInEnsemble` required "
                    "for derived ensemble forecast",
                    Here());
            }

            const long numberOfForecastsInEnsemble =
                grib.getLong("numberOfForecastsInEnsemble");

            misc.set("numberOfForecastsInEnsemble",
                     numberOfForecastsInEnsemble);

            return;
        }

        if (!grib.has("number") || !grib.has("numberOfForecastsInEnsemble")) {
            return;
        }

        const long number = grib.getLong("number");
        const long numberOfForecastsInEnsemble =
            grib.getLong("numberOfForecastsInEnsemble");

        if (number != 0 && numberOfForecastsInEnsemble == 0) {
            throw Grib2MarsGenericException(
                "The value for key `numberOfForecastsInEnsemble` must not be 0",
                Here());
        }

        if (numberOfForecastsInEnsemble == 0) {
            return;
        }

        set_or_throw<long>(mars, keyword, number);

        misc.set("numberOfForecastsInEnsemble",
                 numberOfForecastsInEnsemble);

        if (grib.has("class")) {
            const std::string klass = grib.getString("class");

            if (klass == "ai") {
                // Handled in the encoder, matching the original tool logic.
                return;
            }
        }

        if (grib.has("typeOfEnsembleForecast")) {
            const long typeOfEnsembleForecast =
                grib.getLong("typeOfEnsembleForecast");

            misc.set("typeOfEnsembleForecast", typeOfEnsembleForecast);

            return;
        }

        if (grib.has("eps")) {
            const long eps = grib.getLong("eps");

            misc.set("typeOfEnsembleForecast", eps);
        }
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException(
                "Failed to extract MARS keyword `" + keyword + "`",
                Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl