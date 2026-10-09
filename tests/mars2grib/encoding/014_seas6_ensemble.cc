/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"

std::vector<double> getVals() {
    return std::vector<double>(542080, 0.0);
}

eckit::LocalConfiguration getMars(const std::string& klass, const std::string& stream, const int number) {
    eckit::LocalConfiguration mars;
    mars.set("class", klass);
    mars.set("stream", stream);
    mars.set("type", "fc");
    mars.set("expver", "0001");
    mars.set("grid", "N320");
    mars.set("packing", "ccsds");
    mars.set("param", 78);
    mars.set("system", 6);
    mars.set("number", number);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'10'06);
    mars.set("time", 00'00);
    mars.set("step", 0);
    mars.set("timespan", "none");
    return mars;
}

eckit::LocalConfiguration getMisc() {
    eckit::LocalConfiguration misc;
    misc.set("numberOfForecastsInEnsemble", 101);
    return misc;
}

CASE("encoding") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const std::vector classList{{"od", "rd", "c3"}};
    const std::vector streamList{{"sfdd", "sfmd", "shdd", "shmd", "sfdp", "shdp"}};
    const std::vector numberList{{0, 1, 2, 99, 100}};  // Not all, because that's kinda slow

    for (const auto klass : classList) {
        for (const auto stream : streamList) {
            for (const auto number : numberList) {
                std::cout << "number = " << number << std::endl;
                const auto handle = encoder.encode(getVals(), getMars(klass, stream, number), getMisc());

                // typeOfEnsembleForecast = 8 (Model physics perturbations) or
                //                          9 (Initial conditions and model physics perturbations)
                const auto expectedTOEF = number == 0 ? 8 : 9;

                // MARS
                EXPECT_EQUAL(handle->getString("class"), klass);
                EXPECT_EQUAL(handle->getString("stream"), stream);
                EXPECT_EQUAL(handle->getString("type"), "fc");
                EXPECT_EQUAL(handle->getLong("number"), number);

                // GRIB
                EXPECT_EQUAL(handle->getLong("perturbationNumber"), number);
                EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 101);
                EXPECT_EQUAL(handle->getLong("typeOfGeneratingProcess"), 4);            // Ensemble Forecast
                EXPECT_EQUAL(handle->getLong("typeOfEnsembleForecast"), expectedTOEF);  // See note
                EXPECT_EQUAL(handle->getLong("typeOfProcessedData"), 4);                // Perturbed Forecast
            }
        }
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
