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
#include <string>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

eckit::LocalConfiguration enfhRequest(const std::string& type, long param, const std::string& timespan = "") {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "enfh");
    mars.set("type", type);
    if (type == "pf") {
        mars.set("number", 4);
    }
    mars.set("expver", "0001");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'10'01);
    mars.set("hdate", 2020'10'01);
    mars.set("time", 00'00'00);
    mars.set("step", 24);
    if (!timespan.empty()) {
        mars.set("timespan", timespan);
    }
    return mars;
}

template <typename Handle>
void expectReforecastDates(const Handle& handle) {
    // Identification section carries the hindcast date, PDS the model version date
    EXPECT_EQUAL(handle->getLong("year"), 2020);
    EXPECT_EQUAL(handle->getLong("YearOfModelVersion"), 2026);
    EXPECT_EQUAL(handle->getLong("MonthOfModelVersion"), 10);
    EXPECT_EQUAL(handle->getLong("DayOfModelVersion"), 1);
    EXPECT_EQUAL(handle->getLong("hdate"), 2020'10'01);
}

template <typename F>
void run(F&& f) {
    try {
        f();
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

}  // namespace

CASE("enfh pf non-chemical param stays on the reforecast template") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 280.0), enfhRequest("pf", 167));

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 60);
        expectReforecastDates(handle);
    });
}

CASE("enfh pf instantaneous CO2 flux is a chemical reforecast (PDT 152)") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 1.0e-6), enfhRequest("pf", 228083));

        EXPECT_EQUAL(handle->getLong("paramId"), 228083);
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 152);
        EXPECT_EQUAL(handle->getLong("constituentType"), 3);
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 4);
        expectReforecastDates(handle);
    });
}

CASE("enfh pf accumulated CO2 is a chemical reforecast (PDT 153)") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 1.0e-3), enfhRequest("pf", 228080, "fs"));

        EXPECT_EQUAL(handle->getLong("paramId"), 228080);
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 153);
        EXPECT_EQUAL(handle->getLong("constituentType"), 3);
        EXPECT_EQUAL(handle->getLong("typeOfStatisticalProcessing"), 1);  // Accumulation
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 4);
        expectReforecastDates(handle);
    });
}

CASE("enfh cf instantaneous CO2 flux is a chemical reforecast (PDT 152)") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 1.0e-6), enfhRequest("cf", 228084));

        EXPECT_EQUAL(handle->getLong("paramId"), 228084);
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 152);
        EXPECT_EQUAL(handle->getLong("constituentType"), 3);
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 0);
        expectReforecastDates(handle);
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
