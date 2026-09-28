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

CASE("1/1") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "1/1");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'10);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(65160, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 0L);  // Latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAParallel"), 360L);  // (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 181L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 90'000000L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 48L);       // 0011 0000  Di and Dj given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -90'000000L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'000000L);  // Unit 10^-6 degrees (Lo2)
    EXPECT_EQUAL(handle->getLong("iDirectionIncrement"), 1'000000L);         // Unit 10^-6 degrees (Di)
    EXPECT_EQUAL(handle->getLong("jDirectionIncrement"), 1'000000L);         // Unit 10^-6 degrees (Dj)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 65160);
}

CASE("0.25/0.25") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "0.25/0.25");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'10);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(1038240, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 0L);  // Latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAParallel"), 1440L);  // (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 721L);   // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 90'000000L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 48L);       // 0011 0000  Di and Dj given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -90'000000L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'750000L);  // Unit 10^-6 degrees (Lo2)
    EXPECT_EQUAL(handle->getLong("iDirectionIncrement"), 250000L);           // Unit 10^-6 degrees (Di)
    EXPECT_EQUAL(handle->getLong("jDirectionIncrement"), 250000L);           // Unit 10^-6 degrees (Dj)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 1038240);
}

CASE("0.1/0.1") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "0.1/0.1");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'10);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(1639680, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 0L);  // Latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAParallel"), 360'0L);  // (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 180'1L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 90'000000L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 48L);       // 0011 0000  Di and Dj given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -90'000000L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'900000L);  // Unit 10^-6 degrees (Lo2)
    EXPECT_EQUAL(handle->getLong("iDirectionIncrement"), 100000L);           // Unit 10^-6 degrees (Di)
    EXPECT_EQUAL(handle->getLong("jDirectionIncrement"), 100000L);           // Unit 10^-6 degrees (Dj)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 1639680);
}

CASE("2/1 (square earth)") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "2/1");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'10);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(32580, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 0L);  // Latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAParallel"), 180L);  // (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 181L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 90'000000L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 48L);       // 0011 0000  Di and Dj given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -90'000000L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 358'000000L);  // Unit 10^-6 degrees (Lo2)
    EXPECT_EQUAL(handle->getLong("iDirectionIncrement"), 2'000000L);         // Unit 10^-6 degrees (Di)
    EXPECT_EQUAL(handle->getLong("jDirectionIncrement"), 1'000000L);         // Unit 10^-6 degrees (Dj)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 32580);
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
