/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// Section 4 recipe selection (matchers + recipes only, no encoding) for quantile, post-processed,
/// reference-period and probability products. MARS requests are given as mars2mars hands them to mars2grib
/// (step range -> step + timespan).

#include <exception>
#include <string>

// Dictionary traits first: the backend headers rely on them being declared
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_options.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/frontend/make_HeaderLayout.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

eckit::LocalConfiguration request(const std::string& stream, const std::string& type, long param,
                                  const std::string& levtype, long step, const std::string& timespan) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", stream);
    mars.set("type", type);
    mars.set("expver", "0001");
    mars.set("domain", "g");
    mars.set("grid", "O320");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", levtype);
    if (levtype == "pl") {
        mars.set("levelist", 850);
    }
    mars.set("date", 2026'09'30);
    mars.set("time", 12'00);
    mars.set("step", step);
    mars.set("timespan", timespan);
    return mars;
}

eckit::LocalConfiguration with(eckit::LocalConfiguration mars, const std::string& key, const std::string& value) {
    mars.set(key, value);
    return mars;
}

eckit::LocalConfiguration with(eckit::LocalConfiguration mars, const std::string& key, long value) {
    mars.set(key, value);
    return mars;
}

long section4Template(const eckit::LocalConfiguration& mars) {
    try {
        const metkit::mars2grib::Options opt{};
        const auto layout = metkit::mars2grib::frontend::make_HeaderLayout_or_throw(mars, opt);
        return static_cast<long>(layout.sectionLayouts[4].templateNumber);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        throw;
    }
}

}  // namespace

// --- quantile / post-processing -----------------------------------------------------------------------------------

CASE("pfc: post-processed quantile, accumulated (PDT 90)") {
    EXPECT_EQUAL(section4Template(with(request("enfo", "pfc", 228, "sfc", 132, "6h"), "quantile", "1:100")), 90);
}

CASE("quantile, point in time (PDT 86)") {
    EXPECT_EQUAL(section4Template(with(request("enfo", "pd", 167, "sfc", 24, "none"), "quantile", "50:100")), 86);
}

CASE("quantile, accumulated (PDT 87)") {
    EXPECT_EQUAL(section4Template(with(request("enfo", "pd", 228, "sfc", 24, "24h"), "quantile", "50:100")), 87);
}

// --- reference period: EFI / SOT ----------------------------------------------------------------------------------

CASE("efi and efic (PDT 107)") {
    EXPECT_EQUAL(section4Template(request("enfo", "efi", 132045, "sfc", 360, "360h")), 107);
    EXPECT_EQUAL(section4Template(request("enfo", "efic", 132167, "sfc", 120, "24h")), 107);
}

CASE("sot: quantile holds the reference-period percentiles (PDT 107)") {
    EXPECT_EQUAL(section4Template(with(request("enfo", "sot", 132144, "sfc", 96, "24h"), "quantile", "90-99:100")),
                 107);
    EXPECT_EQUAL(section4Template(with(request("eefo", "sot", 132167, "sfc", 288, "168h"), "quantile", "1-10:100")),
                 107);
}

// --- probabilities ------------------------------------------------------------------------------------------------

CASE("ep threshold probabilities (PDT 5 / 9)") {
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131060, "sfc", 24, "24h")), 9);
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131070, "sfc", 24, "24h")), 9);
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131068, "sfc", 120, "none")), 5);
}

CASE("ep probabilities defined without a statistic: instant (PDT 5), time window maximum (PDT 9)") {
    for (long param : {131068L, 131073L, 131074L, 131075L, 131081L}) {
        EXPECT_EQUAL(section4Template(request("enfo", "ep", param, "sfc", 120, "none")), 5);
        EXPECT_EQUAL(section4Template(request("enfo", "ep", param, "sfc", 168, "48h")), 9);
    }
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131072, "sfc", 24, "24h")), 9);
}

CASE("ep strike probabilities (PDT 121 instant, PDT 122 time window)") {
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131089, "sfc", 216, "none")), 121);
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131089, "sfc", 216, "48h")), 122);
}

CASE("ep probabilities of standardised anomalies (PDT 131 instant, PDT 112 time window)") {
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 133093, "pl", 120, "none")), 131);
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 133096, "pl", 120, "none")), 131);
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 133093, "pl", 120, "24h")), 112);
}

CASE("ep probabilities of anomalies (PDT 131 / 112)") {
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131022, "pl", 120, "none")), 131);
    EXPECT_EQUAL(section4Template(request("enfo", "ep", 131020, "pl", 168, "48h")), 112);
    EXPECT_EQUAL(section4Template(request("eefo", "ep", 131001, "sfc", 168, "168h")), 112);
    EXPECT_EQUAL(section4Template(request("eefo", "ep", 131006, "sfc", 168, "168h")), 112);
}

// --- anomaly / significance parameters (eefo, weekly means) -------------------------------------------------------

CASE("eefo fcmean anomaly: individual ensemble member (PDT 106)") {
    EXPECT_EQUAL(section4Template(with(request("eefo", "fcmean", 171167, "sfc", 168, "168h"), "number", 5L)), 106);
    EXPECT_EQUAL(section4Template(with(request("eefo", "fcmean", 173228, "sfc", 168, "168h"), "number", 5L)), 106);
    EXPECT_EQUAL(section4Template(with(request("eefo", "fcmean", 171129, "pl", 168, "168h"), "number", 5L)), 106);
}

CASE("eefo taem anomaly / significance: derived (PDT 107)") {
    EXPECT_EQUAL(section4Template(request("eefo", "taem", 171167, "sfc", 168, "168h")), 107);
    EXPECT_EQUAL(section4Template(request("eefo", "taem", 234228, "sfc", 168, "168h")), 107);
}

CASE("eefo pb: quantiles of anomalies (PDT 134)") {
    EXPECT_EQUAL(section4Template(with(request("eefo", "pb", 171167, "sfc", 168, "168h"), "quantile", "1:3")), 134);
    EXPECT_EQUAL(section4Template(with(request("eefo", "pb", 171129, "pl", 168, "168h"), "quantile", "2:5")), 134);
}

// --- unchanged ----------------------------------------------------------------------------------------------------

CASE("unchanged: ensemble members and derived products") {
    EXPECT_EQUAL(section4Template(with(request("enfo", "pf", 167, "sfc", 24, "none"), "number", 1L)), 1);
    EXPECT_EQUAL(section4Template(with(request("enfo", "pf", 228, "sfc", 24, "24h"), "number", 1L)), 11);
    EXPECT_EQUAL(section4Template(request("enfo", "em", 167, "sfc", 24, "none")), 2);
    EXPECT_EQUAL(section4Template(request("enfo", "es", 228, "sfc", 24, "24h")), 12);
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
