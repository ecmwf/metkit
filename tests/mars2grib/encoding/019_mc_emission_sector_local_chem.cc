// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// Emission fluxes by sector (class=mc): the ECMWF-local sector params (412000, 463000, 465000, ...) set
/// localTablesVersion 2 and a local source/sink code; a local chem (e.g. 99, 100, 919) needs localTablesVersion 1.
/// chemId is set after paramId, so both decode, as in the CAMS reanalysis (class=a5).

#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"

namespace {

eckit::LocalConfiguration mars(long param, long chem) {
    eckit::LocalConfiguration mars;
    mars.set("class", "mc");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("chem", chem);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'11'21);
    mars.set("time", 00'00);
    mars.set("step", 3);
    mars.set("timespan", "none");
    return mars;
}

const std::vector<double>& values() {
    static const std::vector<double> vals(213988, 1.0e-12);  // N200
    return vals;
}

}  // namespace

CASE("emission sector with an ECMWF-local definition and a local chem") {
    const struct test_t {
        long param;
        long chem;
        long sourceSink;
        long constituentType;
        long localTablesVersion;
    } tests[]{
        {463000, 919, 197, 65511, 1},  // commercial buildings heating, biogenic SOA precursor
        {463000, 99, 197, 65529, 1},   // commercial buildings heating, local chem 99
        {465000, 100, 199, 65530, 1},  // oil refineries and transformation industry, local chem 100
        {463000, 2, 197, 4, 2},        // commercial buildings heating, carbon monoxide (WMO chem)
        {422000, 919, 18, 65511, 1},   // residential, commercial and other combustion (WMO param)
    };
    for (const auto& t : tests) {
        SECTION(std::to_string(t.param) + " chem " + std::to_string(t.chem)) {
            const auto handle = metkit::mars2grib::Mars2Grib().encode(values(), mars(t.param, t.chem));
            EXPECT_EQUAL(handle->getLong("paramId"), t.param);
            EXPECT_EQUAL(handle->getLong("chemId"), t.chem);
            EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 76);
            EXPECT_EQUAL(handle->getLong("sourceSinkChemicalPhysicalProcess"), t.sourceSink);
            EXPECT_EQUAL(handle->getLong("constituentType"), t.constituentType);
            EXPECT_EQUAL(handle->getLong("localTablesVersion"), t.localTablesVersion);
        }
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
