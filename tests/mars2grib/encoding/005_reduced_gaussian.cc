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
#include <exception>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

CASE("N32") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        mars.set("grid", "N32");
        mars.set("packing", "ccsds");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'14);
        mars.set("time", 00'00);
        mars.set("step", 0);

        std::vector<double> vals(6114, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS
        EXPECT_EQUAL(handle->getString("gridName"), "N32");

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Latitude/longitude

        EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
        EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

        EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));           // Missing for reduced gaussian grids (Ni)
        EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 64L);  // (Nj)
        EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
        EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

        EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 87'863799L);   // Unit 10^-6 degrees (La1)
        EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
        EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
        EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -87'863799L);   // Unit 10^-6 degrees (La2)
        EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 357'187500L);  // Unit 10^-6 degrees (Lo2)
        EXPECT(handle->isMissing("iDirectionIncrement"));  // Missing for reduced gaussian grids (Di)
        EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 32);  // (N)

        EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 6114);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

CASE("O32") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        mars.set("grid", "O32");
        mars.set("packing", "ccsds");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'14);
        mars.set("time", 00'00);
        mars.set("step", 0);

        std::vector<double> vals(5248, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS
        EXPECT_EQUAL(handle->getString("gridName"), "O32");

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Latitude/longitude

        EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
        EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

        EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));           // Missing for reduced gaussian grids (Ni)
        EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 64L);  // (Nj)
        EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
        EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

        EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 87'863799L);   // Unit 10^-6 degrees (La1)
        EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
        EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
        EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -87'863799L);   // Unit 10^-6 degrees (La2)
        EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 357'500000L);  // Unit 10^-6 degrees (Lo2)
        EXPECT(handle->isMissing("iDirectionIncrement"));  // Missing for reduced gaussian grids (Di)
        EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 32);  // (N)

        EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 5248);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

CASE("N1280") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        mars.set("grid", "N1280");
        mars.set("packing", "ccsds");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'14);
        mars.set("time", 00'00);
        mars.set("step", 0);

        std::vector<double> vals(8505906, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS
        EXPECT_EQUAL(handle->getString("gridName"), "N1280");

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Gaussian latitude/longitude

        EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
        EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

        EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));  // Missing for reduced gaussian grids (Ni)
        EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 2560L);  // (Nj)
        EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
        EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

        EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 89'946188L);   // Unit 10^-6 degrees (La1)
        EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
        EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
        EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -89'946188L);   // Unit 10^-6 degrees (La2)
        EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'929688L);  // Unit 10^-6 degrees (Lo2)
        EXPECT(handle->isMissing("iDirectionIncrement"));  // Missing for reduced gaussian grids (Di)
        EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 1280);  // (N)

        EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 8505906);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

CASE("O1280") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        mars.set("grid", "O1280");
        mars.set("packing", "ccsds");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'14);
        mars.set("time", 00'00);
        mars.set("step", 0);

        std::vector<double> vals(6599680, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS
        EXPECT_EQUAL(handle->getString("gridName"), "O1280");

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Gaussian latitude/longitude

        EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
        EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

        EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));  // Missing for reduced gaussian grids (Ni)
        EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 2560L);  // (Nj)
        EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
        EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

        EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 89'946188L);   // Unit 10^-6 degrees (La1)
        EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
        EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
        EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -89'946188L);   // Unit 10^-6 degrees (La2)
        EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'929907L);  // Unit 10^-6 degrees (Lo2)
        EXPECT(handle->isMissing("iDirectionIncrement"));  // Missing for reduced gaussian grids (Di)
        EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 1280);  // (N)

        EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 6599680);
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
