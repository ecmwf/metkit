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

static const bool useGridSpec = []() {
    const auto* value = ::getenv("ECCODES_ECKIT_GEO");
    if (!value) {
        return false;
    }
    const std::string stringValue(value);
    return stringValue == "1" || stringValue == "2";
}();

CASE("T1279") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        // mars.set("origin", "ecmf");
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        if (useGridSpec) {
            mars.set("grid", "T1279");
        }
        else {
            mars.set("truncation", 1279);
        }
        mars.set("packing", "complex");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'09);
        mars.set("time", 00'00'00);
        mars.set("step", 0);

        std::vector<double> vals(1639680, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 50L);  // Spherical harmonic coefficients
        EXPECT_EQUAL(handle->getLong("J"), 1279);
        EXPECT_EQUAL(handle->getLong("K"), 1279);
        EXPECT_EQUAL(handle->getLong("M"), 1279);
        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 1639680);

        EXPECT_EQUAL(handle->getLong("dataRepresentationTemplateNumber"), 51L);  // Spherical harmonic data
        EXPECT_EQUAL(handle->getLong("JS"), 20);
        EXPECT_EQUAL(handle->getLong("KS"), 20);
        EXPECT_EQUAL(handle->getLong("MS"), 20);
        EXPECT_EQUAL(handle->getLong("TS"), 462);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

CASE("T1279 (custom subset)") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        // mars.set("origin", "ecmf");
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        if (useGridSpec) {
            mars.set("grid", "T1279");
        }
        else {
            mars.set("truncation", 1279);
        }
        mars.set("packing", "complex");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'09);
        mars.set("time", 00'00'00);
        mars.set("step", 0);

        eckit::LocalConfiguration misc;
        misc.set("subSetTruncation", 42);

        std::vector<double> vals(1639680, 237.15);

        const auto handle = encoder.encode(vals, mars, misc);

        // MARS

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 50L);  // Spherical harmonic coefficients
        EXPECT_EQUAL(handle->getLong("J"), 1279);
        EXPECT_EQUAL(handle->getLong("K"), 1279);
        EXPECT_EQUAL(handle->getLong("M"), 1279);
        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 1639680);

        EXPECT_EQUAL(handle->getLong("dataRepresentationTemplateNumber"), 51L);  // Spherical harmonic data
        EXPECT_EQUAL(handle->getLong("JS"), 42);
        EXPECT_EQUAL(handle->getLong("KS"), 42);
        EXPECT_EQUAL(handle->getLong("MS"), 42);
        EXPECT_EQUAL(handle->getLong("TS"), 1892);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

CASE("T20") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        if (useGridSpec) {
            mars.set("grid", "T20");
        }
        else {
            mars.set("truncation", 20);
        }
        mars.set("packing", "complex");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'09);
        mars.set("time", 00'00'00);
        mars.set("step", 0);

        std::vector<double> vals(462, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 50L);  // Spherical harmonic coefficients
        EXPECT_EQUAL(handle->getLong("J"), 20);
        EXPECT_EQUAL(handle->getLong("K"), 20);
        EXPECT_EQUAL(handle->getLong("M"), 20);
        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 462);

        EXPECT_EQUAL(handle->getLong("dataRepresentationTemplateNumber"), 51L);  // Spherical harmonic data
        EXPECT_EQUAL(handle->getLong("JS"), 10);
        EXPECT_EQUAL(handle->getLong("KS"), 10);
        EXPECT_EQUAL(handle->getLong("MS"), 10);
        EXPECT_EQUAL(handle->getLong("TS"), 132);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

CASE("T20 (no subset)") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        if (useGridSpec) {
            mars.set("grid", "T20");
        }
        else {
            mars.set("truncation", 20);
        }
        mars.set("packing", "complex");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'09);
        mars.set("time", 00'00'00);
        mars.set("step", 0);

        eckit::LocalConfiguration misc;
        misc.set("subSetTruncation", 20);

        std::vector<double> vals(462, 237.15);

        const auto handle = encoder.encode(vals, mars, misc);

        // MARS

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 50L);  // Spherical harmonic coefficients
        EXPECT_EQUAL(handle->getLong("J"), 20);
        EXPECT_EQUAL(handle->getLong("K"), 20);
        EXPECT_EQUAL(handle->getLong("M"), 20);
        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 462);

        EXPECT_EQUAL(handle->getLong("dataRepresentationTemplateNumber"), 51L);  // Spherical harmonic data
        EXPECT_EQUAL(handle->getLong("JS"), 20);
        EXPECT_EQUAL(handle->getLong("KS"), 20);
        EXPECT_EQUAL(handle->getLong("MS"), 20);
        EXPECT_EQUAL(handle->getLong("TS"), 462);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

CASE("T1 (no subset)") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        // mars.set("origin", "ecmf");
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        if (useGridSpec) {
            mars.set("grid", "T1");
        }
        else {
            mars.set("truncation", 1);
        }
        mars.set("packing", "complex");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'09);
        mars.set("time", 00'00'00);
        mars.set("step", 0);

        std::vector<double> vals(6, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 50L);  // Spherical harmonic coefficients
        EXPECT_EQUAL(handle->getLong("J"), 1);
        EXPECT_EQUAL(handle->getLong("K"), 1);
        EXPECT_EQUAL(handle->getLong("M"), 1);
        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 6);

        EXPECT_EQUAL(handle->getLong("dataRepresentationTemplateNumber"), 51L);  // Spherical harmonic data
        EXPECT_EQUAL(handle->getLong("JS"), 1);
        EXPECT_EQUAL(handle->getLong("KS"), 1);
        EXPECT_EQUAL(handle->getLong("MS"), 1);
        EXPECT_EQUAL(handle->getLong("TS"), 6);
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
