/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <exception>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

CASE("stream=oper,type=fc as control forecast") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("origin", "ecmf");
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "0001");
        mars.set("grid", "N200");
        mars.set("packing", "ccsds");
        mars.set("param", 130);
        mars.set("levtype", "hl");
        mars.set("levelist", 2);
        mars.set("date", 2026'09'03);
        mars.set("time", 00'00'00);
        mars.set("step", 0);

        std::vector<double> vals(200, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS
        EXPECT_EQUAL(handle->getString("type"), "fc");
        EXPECT_EQUAL(handle->getLong("number"), 0);

        // GRIB
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 0);
        EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);
        EXPECT_EQUAL(handle->getLong("typeOfGeneratingProcess"), 4);  // Ensemble Forecast
        EXPECT_EQUAL(handle->getLong("typeOfEnsembleForecast"), 5);   // Unpertubed Forecast
        EXPECT_EQUAL(handle->getLong("typeOfProcessedData"), 3);      // Control Forecast
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
