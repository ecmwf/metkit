/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// CAMS (class=mc) params from the 212xxx/216xxx tables: emission fluxes by sector, aerosol optics, tropospheric
// columns.

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


eckit::LocalConfiguration getMars(long param, long chem, long wavelength = 0) {
    eckit::LocalConfiguration mars;
    mars.set("class", "mc");
    mars.set("stream", "oper");
    mars.set("type", "an");
    mars.set("expver", "test");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("chem", chem);
    if (wavelength != 0) {
        mars.set("wavelength", wavelength);
    }
    mars.set("levtype", "sfc");
    mars.set("date", 2026'10'06);
    mars.set("time", 00'00);
    mars.set("step", 0);
    mars.set("timespan", "none");
    return mars;
}

CASE("emission flux from power generation (WMO definition)") {
    const auto handle = metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0), getMars(423000, 12));

    EXPECT_EQUAL(handle->getLong("paramId"), 423000);
    EXPECT_EQUAL(handle->getLong("chemId"), 12);
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 76);
    EXPECT_EQUAL(handle->getLong("sourceSinkChemicalPhysicalProcess"), 19);
}

CASE("emission flux from residential heating: WMO source code, not the ECMWF-local one") {
    const auto handle = metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0), getMars(464000, 19));

    EXPECT_EQUAL(handle->getLong("paramId"), 464000);
    EXPECT_EQUAL(handle->getLong("chemId"), 19);
    EXPECT_EQUAL(handle->getLong("sourceSinkChemicalPhysicalProcess"), 31);
    EXPECT_EQUAL(handle->getLong("localTablesVersion"), 0);
}

CASE("emission flux from residential heating of a local constituent (lumped aromatics)") {
    const auto handle = metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0), getMars(464000, 99));

    EXPECT_EQUAL(handle->getLong("paramId"), 464000);
    EXPECT_EQUAL(handle->getLong("chemId"), 99);
    EXPECT_EQUAL(handle->getLong("sourceSinkChemicalPhysicalProcess"), 31);
    EXPECT_EQUAL(handle->getLong("localTablesVersion"), 1);
}

CASE("aerosol optical depth at 10 um") {
    const auto handle = metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0), getMars(457000, 922, 10000));

    EXPECT_EQUAL(handle->getLong("paramId"), 457000);
    EXPECT_EQUAL(handle->getLong("chemId"), 922);
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 48);
    EXPECT_EQUAL(handle->getLong("wavelength"), 10000);
}

CASE("tropospheric column of ozone") {
    const auto handle = metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0), getMars(473000, 236));

    EXPECT_EQUAL(handle->getLong("paramId"), 473000);
    EXPECT_EQUAL(handle->getLong("chemId"), 236);
    EXPECT_EQUAL(handle->getLong("typeOfSecondFixedSurface"), 7);
}


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
