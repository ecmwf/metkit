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

        if (type == "efi" || type == "efic" || type == "sot" || type == "ep") {
            // Ensemble size and model climate of the index products (GRIB1 local definition 19).
            for (const char* key : {"numberOfForecastsInEnsemble", "numberOfReforecastYearsInModelClimate",
                                    "numberOfDaysInClimateSamplingWindow", "sampleSizeOfModelClimate"}) {
                if (grib.has(key)) {
                    misc.set(key, grib.getLong(key));
                }
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

        // Cluster Mean / Cluster Representative
        if (type == "cm" || type == "cr") {
            const bool g1 = grib.getLong("edition") == 1;

            const auto clusteringStartTimeStep = grib.getLong(g1 ? "startTimeStep" : "clusteringStartTimeStep");
            const auto clusteringEndTimeStep   = grib.getLong(g1 ? "endTimeStep" : "clusteringEndTimeStep");
            const auto representativeMember    = grib.getLong("representativeMember");
            auto climatologicalRegime          = grib.getLong("climatologicalRegime");
            const auto NL                      = grib.getLong(g1 ? "controlForecastCluster" : "NL");
            const auto totalNumberOfClusters   = grib.getLong("totalNumberOfClusters");
            const auto numberOfForecastsInTheCluster =
                grib.getLong(g1 ? "numberOfForecastsInCluster" : "numberOfForecastsInTheCluster");
            const auto ensembleForecastNumbers =
                numberOfForecastsInTheCluster > 0 ? grib.getLongArray("ensembleForecastNumbers") : std::vector<long>{};

            if (climatologicalRegime == 0) {
                climatologicalRegime = 255;
            }

            misc.set("typeOfProcessedData", 5);

            misc.set("clusteringStartTimeStep", clusteringStartTimeStep);
            misc.set("clusteringEndTimeStep", clusteringEndTimeStep);
            misc.set("representativeMember", representativeMember);
            misc.set("climatologicalRegime", climatologicalRegime);
            misc.set("NL", NL);
            misc.set("totalNumberOfClusters", totalNumberOfClusters);
            misc.set("numberOfForecastsInTheCluster", numberOfForecastsInTheCluster);
            misc.set("ensembleForecastNumbers", ensembleForecastNumbers);
        }
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException("Failed to extract MISC keywords", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl