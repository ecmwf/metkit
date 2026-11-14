/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file mars2mars-test-05.cc
/// @brief Legacy atmospheric composition params are converted to the param/chem split.

#include <exception>
#include <stdexcept>
#include <string>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/mars2mars/api/Mars2Mars.h"

namespace {

eckit::LocalConfiguration converted(const std::string& stream, long param, const std::string& step = "24") {
    eckit::LocalConfiguration mars;
    mars.set("class", "mc");
    mars.set("stream", stream);
    mars.set("type", stream == "gfas" ? "ga" : "fc");
    mars.set("expver", "0001");
    mars.set("date", 20261001L);
    mars.set("time", 0L);
    mars.set("levtype", "sfc");
    mars.set("param", param);
    if (step.find('-') == std::string::npos) {
        mars.set("step", std::stol(step));
    }
    else {
        mars.set("step", step);
    }
    try {
        return metkit::mars2mars::Mars2Mars{}.convert(mars).mars;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Test failed with exception: ") + e.what());
    }
}

}  // namespace

CASE("aerosol optical depth: param, chem and the wavelength in nm") {
    const auto out = converted("oper", 210207);  // Total aerosol optical depth at 550 nm
    EXPECT_EQUAL(out.getLong("param"), 457000);
    EXPECT_EQUAL(out.getLong("chem"), 922);
    EXPECT_EQUAL(out.getDouble("wavelength"), 550.0);
}

CASE("wildfire species flux (GFAS: time mean)") {
    const auto out = converted("gfas", 210080);  // Wildfire flux of carbon dioxide
    EXPECT_EQUAL(out.getLong("param"), 469001);
    EXPECT_EQUAL(out.getLong("chem"), 25);
}

CASE("wildfire species flux over a time window (GFAS ga) is a time mean") {
    const auto out = converted("gfas", 210081, "0-24");  // Wildfire flux of carbon monoxide, daily mean
    EXPECT_EQUAL(out.getLong("param"), 469001);
    EXPECT_EQUAL(out.getLong("chem"), 2);
}

CASE("CO2 fluxes 228080-228085") {
    EXPECT_EQUAL(converted("oper", 228080).getLong("param"), 435002);
    EXPECT_EQUAL(converted("oper", 228083).getLong("param"), 435000);
    EXPECT_EQUAL(converted("oper", 228083).getLong("chem"), 25);
}

CASE("CAMS emission flux by sector (212xxx/216xxx): param = sector, chem = species") {
    const auto ene = converted("oper", 212002);  // Acetaldehyde and higher aldehydes, power generation
    EXPECT_EQUAL(ene.getLong("param"), 423000);
    EXPECT_EQUAL(ene.getLong("chem"), 12);

    const auto res = converted("oper", 216226);  // Condensable gas type 2b (SOG2B), residential
    EXPECT_EQUAL(res.getLong("param"), 464000);
    EXPECT_EQUAL(res.getLong("chem"), 188);

    const auto aviation = converted("oper", 212255);  // Aviation NOx as NO2
    EXPECT_EQUAL(aviation.getLong("param"), 418000);
    EXPECT_EQUAL(aviation.getLong("chem"), 17);
}

CASE("CAMS species chem as in the CAMS tracer table (table_bascoetm5ver6.6_newGribIDs)") {
    const auto mcooh = converted("oper", 212152);  // MCOOH (methacrylic acid), power generation
    EXPECT_EQUAL(mcooh.getLong("param"), 423000);
    EXPECT_EQUAL(mcooh.getLong("chem"), 44);
}

CASE("CAMS cloud and rain pH: potential of hydrogen of cloud water (361) and rain water (360)") {
    EXPECT_EQUAL(converted("oper", 212247).getLong("param"), 476000);
    EXPECT_EQUAL(converted("oper", 212247).getLong("chem"), 361);
    EXPECT_EQUAL(converted("oper", 212248).getLong("chem"), 360);
}

CASE("CAMS 216xxx optics and columns") {
    const auto aod = converted("oper", 216010);  // Fine-mode aerosol optical depth at 10 um
    EXPECT_EQUAL(aod.getLong("param"), 457000);
    EXPECT_EQUAL(aod.getLong("chem"), 923);
    EXPECT_EQUAL(aod.getDouble("wavelength"), 10000.0);

    const auto column = converted("oper", 216033);  // Tropospheric column of ozone
    EXPECT_EQUAL(column.getLong("param"), 473000);
    EXPECT_EQUAL(column.getLong("chem"), 236);
}

CASE("CO2 fluxes keep their legacy paramIds outside class=mc") {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "eefh");
    mars.set("type", "pf");
    mars.set("expver", "0001");
    mars.set("date", 20261007L);
    mars.set("time", 0L);
    mars.set("hdate", 20201007L);
    mars.set("number", 1L);
    mars.set("levtype", "sfc");
    mars.set("param", 228083L);
    mars.set("step", 24L);
    const auto out = metkit::mars2mars::Mars2Mars{}.convert(mars).mars;
    EXPECT_EQUAL(out.getLong("param"), 228083);
    EXPECT(!out.has("chem"));
}

CASE("accumulated deposition from the forecast start is timespan fs") {
    const auto dry = converted("oper", 222033, "0-105");  // Accumulated dry deposition of N2O5
    EXPECT_EQUAL(dry.getLong("param"), 445000);
    EXPECT_EQUAL(dry.getLong("chem"), 33);
    EXPECT_EQUAL(dry.getLong("step"), 105);
    EXPECT_EQUAL(dry.getString("timespan"), "fs");

    // Also when the range happens to match a language timespan
    const auto wet = converted("oper", 223006, "0-24");  // Accumulated wet deposition of HNO3
    EXPECT_EQUAL(wet.getLong("param"), 444000);
    EXPECT_EQUAL(wet.getLong("step"), 24);
    EXPECT_EQUAL(wet.getString("timespan"), "fs");
}

CASE("a time window starting at step 0 keeps its length") {
    const auto out = converted("gfas", 210081, "0-24");
    EXPECT_EQUAL(out.getString("timespan"), "24h");
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
