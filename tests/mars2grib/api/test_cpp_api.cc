// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <exception>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

CASE("mars2grib_api") {
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
        mars.set("date", 2026'02'05);
        mars.set("time", 00'00'00);
        mars.set("step", 0);

        std::vector<double> vals(200, 237.15);

        encoder.encode(vals, mars);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("CPP API test failed", Here()));
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
