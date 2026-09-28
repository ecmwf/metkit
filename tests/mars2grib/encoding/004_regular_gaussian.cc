/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <cstdlib>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"

CASE("F32") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "F32");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(8192, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("gridName"), "F32");

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAParallel"), 128L);  // (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 64L);   // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 87'863799L);  // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);  // Unit 10^-6 degrees (Lo1)
    // ECC-2336: i/jDirectionIncrementGiven flags are currently not correctly encoded in eccodes with gridSpec
    // EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 32L);                // 0010 0000  Di given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -87'863799L);            // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 357'187500L);           // Unit 10^-6 degrees (Lo2)
    EXPECT_EQUAL(handle->getLong("iDirectionIncrement"), 2'812500L);                  // Unit 10^-6 degrees (Di)
    EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 32);  // (N)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 8192);
}

CASE("F1280") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "F1280");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(13107200, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("gridName"), "F1280");

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Gaussian latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAParallel"), 5120L);  // (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 2560L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 89'946188L);  // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);  // Unit 10^-6 degrees (Lo1)
    // ECC-2336: i/jDirectionIncrementGiven flags are currently not correctly encoded in eccodes with gridSpec
    // EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 32L);                // 0010 0000  Di given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -89'946188L);              // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'929688L);             // Unit 10^-6 degrees (Lo2)
    EXPECT_EQUAL(handle->getLong("iDirectionIncrement"), 70313L);                       // Unit 10^-6 degrees (Di)
    EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 1280);  // (N)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 13107200);
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
