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

// Wave hindcasts (MARS stream enwh) are encoded as stream enfh, see mars2mars `convertWaveStreams`.
eckit::LocalConfiguration waveRequest(const std::string& stream, const std::string& type, long param) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", stream);
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
    if (stream == "enfh") {
        mars.set("hdate", 2020'10'01);
    }
    mars.set("time", 00'00'00);
    mars.set("step", 24);
    if (param == 140251) {
        mars.set("frequency", 3);
        mars.set("direction", 7);
    }
    return mars;
}

// Direction and frequency grids of the 2D wave spectra
eckit::LocalConfiguration spectraMisc() {
    eckit::LocalConfiguration misc;
    misc.set("numberOfWaveDirections", 36L);
    misc.set("numberOfWaveFrequencies", 29L);
    misc.set("indexOfReferenceWaveFrequency", 1L);
    misc.set("referenceWaveFrequency", 0.03453);
    misc.set("waveFrequencySpacingRatio", 1.1);
    return misc;
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

template <typename Handle>
void expectSpectra(const Handle& handle) {
    EXPECT_EQUAL(handle->getLong("paramId"), 140251);
    EXPECT_EQUAL(handle->getLong("numberOfWaveDirections"), 36);
    EXPECT_EQUAL(handle->getLong("numberOfWaveFrequencies"), 29);
    EXPECT_EQUAL(handle->getLong("waveDirectionNumber"), 7);
    EXPECT_EQUAL(handle->getLong("waveFrequencyNumber"), 3);
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

CASE("enfh pf integrated wave param stays on the reforecast template") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 1.5), waveRequest("enfh", "pf", 140229));

        EXPECT_EQUAL(handle->getLong("paramId"), 140229);
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 60);
        expectReforecastDates(handle);
    });
}

CASE("enfh pf wave 2D spectra is an ensemble reforecast (PDT 142)") {
    run([] {
        auto encoder = metkit::mars2grib::Mars2Grib();
        const auto handle =
            encoder.encode(std::vector<double>(200, 0.1), waveRequest("enfh", "pf", 140251), spectraMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 142);
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 4);
        expectSpectra(handle);
        expectReforecastDates(handle);
    });
}

CASE("enfh cf wave 2D spectra is an ensemble reforecast (PDT 142)") {
    run([] {
        auto encoder = metkit::mars2grib::Mars2Grib();
        const auto handle =
            encoder.encode(std::vector<double>(200, 0.1), waveRequest("enfh", "cf", 140251), spectraMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 142);
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 0);
        expectSpectra(handle);
        expectReforecastDates(handle);
    });
}

CASE("enfh pf wave period range is an ensemble reforecast (PDT 140)") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 0.5), waveRequest("enfh", "pf", 140114));

        EXPECT_EQUAL(handle->getLong("paramId"), 140114);
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 140);
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 4);
        expectReforecastDates(handle);
    });
}

CASE("enfo pf wave 2D spectra stays on the ensemble template (PDT 100)") {
    run([] {
        auto encoder = metkit::mars2grib::Mars2Grib();
        const auto handle =
            encoder.encode(std::vector<double>(200, 0.1), waveRequest("enfo", "pf", 140251), spectraMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 100);
        expectSpectra(handle);
    });
}

CASE("enfo pf wave period range stays on the ensemble template (PDT 104)") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 0.5), waveRequest("enfo", "pf", 140114));

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 104);
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
