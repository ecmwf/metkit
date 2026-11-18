// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// Simulated satellite products carry the channel in Section 4 (templates 4.204/4.205, "with channel, bandwidth
/// and polarisation"), not in the ECMWF local section (local definition 24, or 192 = 24 + 36). Their local
/// section is the ordinary one: local definition 1, or 36 with analysis metadata.

#include <exception>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/grib2mars/api/Grib2Mars.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

const std::vector<double> values(200, 250.0);

// A simulated satellite image (type=ssd) of the deterministic forecast
eckit::LocalConfiguration satelliteRequest(long param) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "ssd");
    mars.set("expver", "0001");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'11'14);
    mars.set("time", 00'00);
    mars.set("step", 6);
    mars.set("channel", 7);
    mars.set("ident", 57);
    mars.set("instrument", 207);
    return mars;
}

// What the satellite deductions need beyond the MARS keys (in a re-encoding it comes from the input header)
eckit::LocalConfiguration satelliteMisc() {
    eckit::LocalConfiguration misc;
    misc.set("satelliteSeries", 333L);
    misc.set("scaleFactorOfCentralWaveNumber", 0L);
    misc.set("scaledValueOfCentralWaveNumber", 136986L);
    return misc;
}

template <typename F>
void run(F&& f) {
    try {
        f();
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        throw;
    }
}

void checkSatelliteSection4(const metkit::codes::CodesHandle& handle) {
    EXPECT_EQUAL(handle.getLong("numberOfContributingSpectralBands"), 1);
    EXPECT_EQUAL(handle.getLong("satelliteSeries"), 333);
    EXPECT_EQUAL(handle.getLong("satelliteNumber"), 57);
    EXPECT_EQUAL(handle.getLong("instrumentType"), 207);
    EXPECT_EQUAL(handle.getLong("channelNumber"), 7);
    EXPECT_EQUAL(handle.getLong("scaleFactorOfCentralWaveNumber"), 0);
    EXPECT_EQUAL(handle.getLong("scaledValueOfCentralWaveNumber"), 136986);
    EXPECT(handle.isMissing("scaleFactorOfBandwidth"));
    EXPECT(handle.isMissing("scaledValueOfBandwidth"));
    EXPECT_EQUAL(handle.getLong("polarisation"), 255);
}

}  // namespace

CASE("a deterministic satellite product uses template 4.204 and local definition 1") {
    run([] {
        auto encoder = metkit::mars2grib::Mars2Grib();
        for (const long param : {260510L, 260511L, 260512L}) {
            const auto handle = encoder.encode(values, satelliteRequest(param), satelliteMisc());

            EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 204);
            EXPECT_EQUAL(handle->getLong("localDefinitionNumber"), 1);
            EXPECT_EQUAL(handle->getLong("forecastTime"), 6);
            EXPECT_EQUAL(handle->getLong("paramId"), param);
            checkSatelliteSection4(*handle);
        }
    });
}

CASE("with analysis metadata the local definition is 36, not 192 (24 + 36)") {
    run([] {
        auto mars = satelliteRequest(260510);
        mars.set("anoffset", 9);

        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(values, mars, satelliteMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 204);
        EXPECT_EQUAL(handle->getLong("localDefinitionNumber"), 36);
        EXPECT_EQUAL(handle->getLong("offsetToEndOf4DvarWindow"), 9);
        checkSatelliteSection4(*handle);
    });
}

// The control forecast: a perturbed member (type=pf) of a satellite product cannot be encoded yet, with these
// templates or with the previous 4.33 (setting the MARS type fails in ecCodes).
CASE("an ensemble member uses template 4.205") {
    run([] {
        auto mars = satelliteRequest(260510);
        mars.set("stream", "enfo");
        mars.set("type", "cf");
        mars.set("number", 0);

        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(values, mars, satelliteMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 205);
        EXPECT_EQUAL(handle->getLong("localDefinitionNumber"), 1);
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 0);
        checkSatelliteSection4(*handle);
    });
}

CASE("brightness temperature of stream=oper is a satellite product: template 4.204, local definition 36") {
    run([] {
        auto mars = satelliteRequest(194);
        mars.set("type", "an");
        mars.set("anoffset", 9);

        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(values, mars, satelliteMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 204);
        EXPECT_EQUAL(handle->getLong("localDefinitionNumber"), 36);
        checkSatelliteSection4(*handle);
    });
}

CASE("brightness temperature of stream=oper without the satellite identification is rejected") {
    auto mars = satelliteRequest(194);
    mars.set("type", "an");
    mars.set("anoffset", 9);
    mars.remove("ident");
    mars.remove("instrument");

    auto encoder = metkit::mars2grib::Mars2Grib();
    EXPECT_THROWS(encoder.encode(values, mars, satelliteMisc()));
}

// The `channel` of the elda brightness temperature is an index over instrument and channel, not a real channel:
// it is not a satellite product and keeps the channel in the local section.
CASE("brightness temperature of stream=elda, type=em: derived forecast template 4.2, channel in local definition 37") {
    run([] {
        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "elda");
        mars.set("type", "em");
        mars.set("expver", "0001");
        mars.set("grid", "N200");
        mars.set("packing", "ccsds");
        mars.set("param", 194);
        mars.set("levtype", "sfc");
        mars.set("date", 2026'11'14);
        mars.set("time", 00'00);
        mars.set("step", 6);
        mars.set("anoffset", 9);
        mars.set("channel", 7);

        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(values, mars, eckit::LocalConfiguration{});

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 2);
        EXPECT_EQUAL(handle->getLong("localDefinitionNumber"), 37);
        EXPECT_EQUAL(handle->getLong("channel"), 7);
        EXPECT_EQUAL(handle->getLong("paramId"), 194);
        EXPECT_EQUAL(handle->getString("mars.type"), "em");
    });
}

CASE("brightness temperature of stream=elda is not a satellite product even with ident and instrument") {
    run([] {
        auto mars = satelliteRequest(194);
        mars.set("stream", "elda");
        mars.set("type", "em");
        mars.set("anoffset", 9);

        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(values, mars, satelliteMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 2);
        EXPECT_EQUAL(handle->getLong("localDefinitionNumber"), 37);
        EXPECT_EQUAL(handle->getLong("channel"), 7);
    });
}

CASE("the channel is recovered from Section 4 when decoding, and the product re-encodes to the same") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(values, satelliteRequest(260510), satelliteMisc());

        auto [mars, misc] = metkit::grib2mars::Grib2Mars().convert<eckit::LocalConfiguration>(*handle);
        EXPECT_EQUAL(mars.getLong("channel"), 7);
        EXPECT_EQUAL(mars.getLong("ident"), 57);
        EXPECT_EQUAL(mars.getLong("instrument"), 207);

        const auto again = encoder.encode(values, mars, misc);
        EXPECT_EQUAL(again->getLong("productDefinitionTemplateNumber"), 204);
        EXPECT_EQUAL(again->getLong("localDefinitionNumber"), 1);
        checkSatelliteSection4(*again);
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
