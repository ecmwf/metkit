/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <cmath>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"

namespace {

constexpr long truncation = 63;

// Spectrum with amplitudes decaying as (n*(n+1))^-1.2, so the fitted laplacian operator is ~1.2
std::vector<double> spectrum() {
    std::vector<double> values;
    values.reserve((truncation + 1) * (truncation + 2));
    for (long m = 0; m <= truncation; ++m) {
        for (long n = m; n <= truncation; ++n) {
            const double amplitude = std::pow(n * (n + 1.0) + 1.0, -1.2) * 1000.0;
            values.push_back(amplitude);
            values.push_back(m == 0 ? 0.0 : 0.5 * amplitude);
        }
    }
    return values;
}

eckit::LocalConfiguration marsRequest(long param, const std::string& levtype) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "0001");
    mars.set("truncation", truncation);
    mars.set("packing", "complex");
    mars.set("param", param);
    mars.set("levtype", levtype);
    mars.set("levelist", levtype == "pl" ? 500 : 1);
    mars.set("date", 2026'02'05);
    mars.set("time", 00'00);
    mars.set("step", 0);
    return mars;
}

}  // namespace

CASE("Fixed laplacian operator for z and lnsp on model levels") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    for (long param : {129, 152}) {
        const auto handle = encoder.encode(spectrum(), marsRequest(param, "ml"));
        EXPECT_EQUAL(handle->getLong("laplacianScalingFactor"), 500000);
    }
}

CASE("Computed laplacian operator for other spectral fields") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    for (const auto& [param, levtype] : {std::pair{138L, "ml"}, std::pair{129L, "pl"}}) {
        const auto handle = encoder.encode(spectrum(), marsRequest(param, levtype));
        EXPECT(std::abs(handle->getDouble("laplacianOperator") - 1.2) < 0.01);
    }
}

CASE("Laplacian operator with a prepared sample shared between params") {
    auto encoder = metkit::mars2grib::Mars2Grib();
    eckit::LocalConfiguration misc;

    const auto lnsp      = marsRequest(152, "ml");
    const auto vorticity = marsRequest(138, "ml");

    const auto lnspEntry = encoder.prepare(lnsp, misc);
    EXPECT(std::abs(encoder.finaliseEncoding(lnspEntry, spectrum(), vorticity, misc)->getDouble("laplacianOperator") -
                    1.2) < 0.01);

    const auto vorticityEntry = encoder.prepare(vorticity, misc);
    EXPECT_EQUAL(encoder.finaliseEncoding(vorticityEntry, spectrum(), lnsp, misc)->getLong("laplacianScalingFactor"),
                 500000);
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
