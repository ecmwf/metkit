// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <cmath>
#include <cstdlib>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


static const std::vector<double> vals() {
    std::vector<double> vals(29040);  // 1.5/1.5
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(46252.1, 58512.7);
    for (auto& v : vals) {
        v = dist(rng);
    }
    return vals;
}

eckit::LocalConfiguration mars(const std::string& type) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", type);
    mars.set("expver", "0001");
    mars.set("grid", "1.5/1.5");
    mars.set("packing", "ccsds");
    mars.set("param", 129);  // Geopotential
    mars.set("levtype", "pl");
    mars.set("levelist", 500);
    mars.set("date", 2026'11'19);
    mars.set("time", 00'00);
    mars.set("step", 84);
    mars.set("timespan", "none");
    mars.set("number", 2);  // Should be in range 1-6
    mars.set("domain", "h");
    return mars;
}

eckit::LocalConfiguration misc() {
    eckit::LocalConfiguration misc;
    misc.set("clusteringStartTimeStep", 72);
    misc.set("clusteringEndTimeStep", 96);
    misc.set("representativeMember", 33);           // Should be in range 0-51
    misc.set("climatologicalRegime", 1);            // Positive phase of the North Atlantic Oscillation (NAO)
    misc.set("NL", 3);                              // clusterIdentifier of the control forecast, should be in range 1-6
    misc.set("totalNumberOfClusters", 3);           // Should be in range 1-6
    misc.set("numberOfForecastsInTheCluster", 15);  // Should be in range 0-51
    misc.set("ensembleForecastNumbers", std::vector<long>({1, 2, 7, 13, 20, 21, 25, 33, 37, 40, 43, 44, 47, 48, 50}));
    return misc;
}

// For more info about this product, see:
// https://www.ecmwf.int/sites/default/files/elibrary/2011/17442-new-clustering-products.pdf
// For expected before/after conversion of this product, see (on ECMWF HPC): /ec/res4/scratch/maro/CM_CR

// Note: The following keys will be missing if clusterIdentifier > totalNumberOfClusters
// representativeMember
// climatologicalRegime
// Key numberOfForecastsInTheCluster will be 0


CASE("type cm Geopotential") {
    const auto handle = metkit::mars2grib::Mars2Grib().encode(vals(), mars("cm"), misc());

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "cm");
    EXPECT_EQUAL(handle->getLong("param"), 129);
    EXPECT_EQUAL(handle->getLong("number"), 2);
    EXPECT_EQUAL(handle->getString("domain"), "h");


    // GRIB2 Local section
    EXPECT_EQUAL(handle->getLong("grib2LocalSectionNumber"), 32);  // Cluster Scenarios
    EXPECT_EQUAL(handle->getLong("clusteringStartTimeStep"), 72);
    EXPECT_EQUAL(handle->getLong("clusteringEndTimeStep"), 96);
    EXPECT_EQUAL(handle->getLong("representativeMember"), 33);

    // See: https://github.com/ecmwf/eccodes/blob/develop/definitions/grib2/tables/local/ecmf/regimes.table
    EXPECT_EQUAL(handle->getLong("climatologicalRegime"), 1);  // Positive phase of the North Atlantic Oscillation (NAO)


    // GRIB2 Product definition section
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 3);  // Derived + rectangular cluster

    EXPECT_EQUAL(handle->getLong("derivedForecast"), 6);               // Unweighted mean of the cluster members
    EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);  // Default

    // See: https://github.com/ecmwf/eccodes/blob/develop/definitions/grib2/templates/template.4.rectangular_cluster.def
    EXPECT_EQUAL(handle->getLong("clusterIdentifier"), 2);  // mars.number
    EXPECT(handle->isMissing("NH"));
    EXPECT_EQUAL(handle->getLong("NL"), 3);  // clusterIdentifier of the control forecast
    EXPECT_EQUAL(handle->getLong("totalNumberOfClusters"), 3);
    EXPECT_EQUAL(handle->getLong("clusteringMethod"), 1);  // Root mean square

    // Next 4 keys are given by mars.domain = h
    EXPECT_EQUAL(handle->getLong("northernLatitudeOfClusterDomain"), 75'000000);
    EXPECT_EQUAL(handle->getLong("southernLatitudeOfClusterDomain"), 30'000000);
    EXPECT_EQUAL(handle->getLong("easternLongitudeOfClusterDomain"), 40'000000);
    EXPECT_EQUAL(handle->getLong("westernLongitudeOfClusterDomain"), 340'000000);

    EXPECT_EQUAL(handle->getLong("numberOfForecastsInTheCluster"), 15);

    EXPECT(handle->isMissing("scaleFactorOfStandardDeviation"));
    EXPECT(handle->isMissing("scaledValueOfStandardDeviation"));
    EXPECT(handle->isMissing("scaleFactorOfDistanceFromEnsembleMean"));
    EXPECT(handle->isMissing("scaledValueOfDistanceFromEnsembleMean"));

    const auto ensembleForecastNumbers = handle->getLongArray("ensembleForecastNumbers");
    EXPECT(ensembleForecastNumbers == std::vector<long>({1, 2, 7, 13, 20, 21, 25, 33, 37, 40, 43, 44, 47, 48, 50}));
}


CASE("type cr Geopotential") {
    const auto handle = metkit::mars2grib::Mars2Grib().encode(vals(), mars("cr"), misc());

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "cr");
    EXPECT_EQUAL(handle->getLong("param"), 129);
    EXPECT_EQUAL(handle->getLong("number"), 2);
    EXPECT_EQUAL(handle->getString("domain"), "h");


    // GRIB2 Local section
    EXPECT_EQUAL(handle->getLong("grib2LocalSectionNumber"), 32);  // Cluster Scenarios
    EXPECT_EQUAL(handle->getLong("clusteringStartTimeStep"), 72);
    EXPECT_EQUAL(handle->getLong("clusteringEndTimeStep"), 96);
    EXPECT_EQUAL(handle->getLong("representativeMember"), 33);

    // See: https://github.com/ecmwf/eccodes/blob/develop/definitions/grib2/tables/local/ecmf/regimes.table
    EXPECT_EQUAL(handle->getLong("climatologicalRegime"), 1);  // Positive phase of the North Atlantic Oscillation (NAO)


    // GRIB2 Product definition section
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 3);  // Derived + rectangular cluster

    EXPECT_EQUAL(handle->getLong("derivedForecast"), 192);             // Forecast of cluster representative member
    EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);  // Default

    // See: https://github.com/ecmwf/eccodes/blob/develop/definitions/grib2/templates/template.4.rectangular_cluster.def
    EXPECT_EQUAL(handle->getLong("clusterIdentifier"), 2);  // mars.number
    EXPECT(handle->isMissing("NH"));
    EXPECT_EQUAL(handle->getLong("NL"), 3);  // clusterIdentifier of the control forecast
    EXPECT_EQUAL(handle->getLong("totalNumberOfClusters"), 3);
    EXPECT_EQUAL(handle->getLong("clusteringMethod"), 1);  // Root mean square

    // Next 4 keys are given by mars.domain = h
    EXPECT_EQUAL(handle->getLong("northernLatitudeOfClusterDomain"), 75'000000);
    EXPECT_EQUAL(handle->getLong("southernLatitudeOfClusterDomain"), 30'000000);
    EXPECT_EQUAL(handle->getLong("easternLongitudeOfClusterDomain"), 40'000000);
    EXPECT_EQUAL(handle->getLong("westernLongitudeOfClusterDomain"), 340'000000);

    EXPECT_EQUAL(handle->getLong("numberOfForecastsInTheCluster"), 15);

    EXPECT(handle->isMissing("scaleFactorOfStandardDeviation"));
    EXPECT(handle->isMissing("scaledValueOfStandardDeviation"));
    EXPECT(handle->isMissing("scaleFactorOfDistanceFromEnsembleMean"));
    EXPECT(handle->isMissing("scaledValueOfDistanceFromEnsembleMean"));

    const auto ensembleForecastNumbers = handle->getLongArray("ensembleForecastNumbers");
    EXPECT(ensembleForecastNumbers == std::vector<long>({1, 2, 7, 13, 20, 21, 25, 33, 37, 40, 43, 44, 47, 48, 50}));
}


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
