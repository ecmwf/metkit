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
#include <string>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

static const bool useGridSpec = []() {
    const auto* value = ::getenv("ECCODES_ECKIT_GEO");
    return value != nullptr && std::stol(value) != 0L;
}();

// ecPoint (weather-type subgrid-calibration) of an individual ensemble member, as in od:0001:enfo:
// type=gbf (bias-corrected gridbox) and type=gwt (weather types) of 6-hourly total precipitation
// (MARS param=228,step=126-132 there; mars2mars turns it into param=228228,step=132,timespan=6)
eckit::LocalConfiguration ecPointRequest(const std::string& type, long number) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", type);
    mars.set("expver", "0001");
    mars.set("domain", "g");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", 228228);
    mars.set("levtype", "sfc");
    mars.set("number", number);
    mars.set("date", 2026'10'13);
    mars.set("time", 00'00);
    mars.set("step", 132);
    mars.set("timespan", 6);
    return mars;
}

// Initial condition perturbation of the ensemble, as in od:0001:enfo (spectral lnsp on model level 1)
eckit::LocalConfiguration icpRequest() {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", "icp");
    mars.set("expver", "0001");
    mars.set("domain", "g");
    if (useGridSpec) {
        mars.set("grid", "T42");
    }
    else {
        mars.set("truncation", 42);
    }
    mars.set("packing", "complex");
    mars.set("param", 152);
    mars.set("levtype", "ml");
    mars.set("levelist", 1);
    mars.set("number", 1);
    mars.set("date", 2026'10'13);
    mars.set("time", 00'00);
    mars.set("step", 0);
    return mars;
}

eckit::LocalConfiguration ensembleMisc() {
    eckit::LocalConfiguration misc;
    misc.set("numberOfForecastsInEnsemble", 51);
    return misc;
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

void checkEcPoint(const std::string& type, long number, long pdt) {
    auto encoder = metkit::mars2grib::Mars2Grib();
    auto mars    = ecPointRequest(type, number);
    if (pdt == 71) {
        mars.set("param", 167);
        mars.set("step", 24);
        mars.set("timespan", "none");
    }
    const auto handle = encoder.encode(std::vector<double>(200, 0.001), mars, ensembleMisc());

    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), pdt);
    EXPECT_EQUAL(handle->getString("mars.type"), type);
    EXPECT_EQUAL(handle->getLong("typeOfProcessedData"), 4);
    const long expectedTypeOfEnsembleForecast = (number == 0) ? 5 : 6;  // unperturbed : perturbed
    EXPECT_EQUAL(handle->getLong("typeOfEnsembleForecast"), expectedTypeOfEnsembleForecast);
    EXPECT_EQUAL(handle->getLong("perturbationNumber"), number);
    EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);
    EXPECT_EQUAL(handle->getLong("typeOfGeneratingProcess"), 13);
    EXPECT_EQUAL(handle->getLong("typeOfPostProcessing"), 206);
    EXPECT_EQUAL(handle->getLong("inputOriginatingCentre"), 98);
    EXPECT_EQUAL(handle->getLong("inputProcessIdentifier"), 16);
    if (pdt == 73) {
        EXPECT_EQUAL(handle->getLong("paramId"), 228228);
        EXPECT_EQUAL(handle->getLong("typeOfStatisticalProcessing"), 1);
        EXPECT_EQUAL(handle->getLong("lengthOfTimeRange"), 6);
        EXPECT_EQUAL(handle->getString("stepRange"), "126-132");
    }
}

}  // namespace

CASE("enfo gbf accumulation is a post-processed ensemble member in a time interval (PDT 73)") {
    run([] { checkEcPoint("gbf", 35, 73); });
}

CASE("enfo gwt accumulation is a post-processed ensemble member in a time interval (PDT 73)") {
    run([] { checkEcPoint("gwt", 35, 73); });
}

CASE("enfo gbf at a point in time is a post-processed ensemble member (PDT 71)") {
    run([] { checkEcPoint("gbf", 1, 71); });
}

CASE("enfo gbf of the control member is unperturbed") {
    run([] { checkEcPoint("gbf", 0, 73); });
}

CASE("enfo icp keeps typeOfEnsembleForecast and typeOfProcessedData missing (PDT 1)") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(1892, 0.0), icpRequest(), ensembleMisc());

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 1);
        EXPECT_EQUAL(handle->getString("mars.type"), "icp");
        EXPECT_EQUAL(handle->getLong("typeOfEnsembleForecast"), 255);
        EXPECT_EQUAL(handle->getLong("typeOfProcessedData"), 255);
        EXPECT_EQUAL(handle->getLong("typeOfGeneratingProcess"), 255);
        EXPECT_EQUAL(handle->getLong("perturbationNumber"), 1);
        EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
