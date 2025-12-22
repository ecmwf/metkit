#pragma once

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/tables/typeOfProcessedData.h"


namespace metkit::mars2grib::backend::deductions {


template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::TypeOfProcessedData resolve_TypeOfProcessed_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                             const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get mars type from dictionary
        std::string marsClass  = get_or_throw<std::string>(mars, "class");
        std::string marsType   = get_or_throw<std::string>(mars, "type");
        std::string marsStream = get_or_throw<std::string>(mars, "stream");

        // Return value
        tables::TypeOfProcessedData result = tables::TypeOfProcessedData::Missing;

        if (has(par, "typeOfProcessedData")) {

            // User override from par dictionary
            long typeOfProcessedDataVal = get_or_throw<long>(par, "typeOfProcessedData");

            // Convert long to enum (validate)
            result = tables::long2enum_TypeOfProcessedData_or_throw(typeOfProcessedDataVal);
        }
        else {
            // Deduce typeOfProcessedData from mars type
            if (marsType == "an") {
                result = tables::TypeOfProcessedData::AnalysisProducts;
            }
            else if (marsType == "fc") {
                result = tables::TypeOfProcessedData::ForecastProducts;
            }
            else if (marsType == "pf") {
                result = tables::TypeOfProcessedData::PerturbedForecastProducts;
            }
            else if (marsType == "cf") {
                result = tables::TypeOfProcessedData::ControlForecastProducts;
            }
            else if (marsType == "ssd" || marsType == "gsd") {
                result = tables::TypeOfProcessedData::ProcessedSatelliteObservations;
            }
            else {
                result = tables::TypeOfProcessedData::Missing;
            }
        }

        // Logging of the deduced typeOfProcessedData
        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`typeOfProcessedData` deduced from MARS dictionary: actual='";
            logMsg += enum2name_TypeOfProcessedData_or_throw(result) + "'";
            return logMsg;
        }());

        // return the deduced type of processed data
        return result;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Unable to get `typeOfProcessedData` from Mars or Parametrization dictionaries", Here()));
    }
};


}  // namespace metkit::mars2grib::backend::deductions
