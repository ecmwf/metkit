// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


CASE("SEAS6 Statistics Encoding Failure") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("expver", "jbiq");
    mars.set("class", "rd");
    mars.set("stream", "sfdd");
    mars.set("type", "fc");
    mars.set("grid", "O200");
    mars.set("packing", "ccsds");
    mars.set("param", 235035);
    mars.set("stattype", "momx");
    mars.set("levtype", "sfc");
    mars.set("levelist", 0);
    mars.set("date", 2001'05'01);
    mars.set("time", 00'00);
    // mars.set("step", 744);
    mars.set("fcmonth", 1);
    mars.set("timespan", 24);
    mars.set("number", 0);
    mars.set("system", 0);
    mars.set("method", 1);
    eckit::LocalConfiguration misc;
    // misc.set("timeIncrementInSeconds", 600);

    std::vector<double> vals(167200, 0);

    const auto handle = encoder.encode(vals, mars, misc);

    // MARS
    EXPECT_EQUAL(handle->getLong("step"), 744);
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
