// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"

CASE("stream=oper,type=fc as control forecast") {
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

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
