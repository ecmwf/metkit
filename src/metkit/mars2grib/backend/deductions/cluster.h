// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file cluster.h
/// @brief Deduction of the cluster-product metadata of the GRIB Product Definition Section.
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// the **cluster metadata** of cluster products (cluster mean and cluster
/// representative).
///
/// All values are taken verbatim from the parameter dictionary (`misc`).
/// No inference, defaulting, or heuristic logic is applied.
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or malformed inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - RESOLVE: value derived via deduction logic from input dictionaries
///
/// @section References
/// Concept:
/// - @ref clusterEncoding.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>
#include <vector>

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Cluster metadata of a cluster product.
///
struct Cluster {
    long clusteringStartTimeStep;
    long clusteringEndTimeStep;
    long representativeMember;
    long climatologicalRegime;
    long clusterIdentifier;
    long NL;  // clusterIdentifier of the control forecast
    long totalNumberOfClusters;
    long clusteringMethod;
    long northernLatitudeOfClusterDomain;
    long southernLatitudeOfClusterDomain;
    long easternLongitudeOfClusterDomain;
    long westernLongitudeOfClusterDomain;
    long numberOfForecastsInTheCluster;
    std::vector<long> ensembleForecastNumbers;
};

///
/// @brief Resolve the cluster metadata from the parameter dictionary.
///
/// @section Deduction contract
/// - Reads: `mars["number"]`, `par["clusteringStartTimeStep"]`, `par["clusteringEndTimeStep"]`,
///   `par["representativeMember"]`, `par["climatologicalRegime"]`, `par["NL"]`,
///   `par["totalNumberOfClusters"]`, `par["numberOfForecastsInTheCluster"]`,
///   `par["ensembleForecastNumbers"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE)
/// - Failure mode: throws
///
/// All keys are required. `clusterIdentifier` is the MARS `number` of the cluster product.
///
/// @tparam MarsDict_t  Type of the MARS dictionary providing `number`.
/// @tparam ParDict_t   Type of the parameter dictionary.
/// @tparam OptDict_t   Type of the options dictionary (unused).
///
/// @param[in] mars  MARS dictionary providing `number` (the cluster identifier).
/// @param[in] par   Parameter dictionary providing the cluster keys.
/// @param[in] opt   Options dictionary (unused).
///
/// @return The resolved cluster metadata.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If any required key is missing or has an unexpected type, or any unexpected
/// error occurs during deduction.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
Cluster resolve_Cluster_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)opt;

    try {

        // Check assumptions for this deductions. Deduction should be refactored if more cases appear in the future.
        {
            const auto marsType   = get_or_throw<std::string>(mars, "type");
            const auto marsParam  = get_or_throw<long>(mars, "param");
            const auto marsDomain = get_or_throw<std::string>(mars, "domain");

            if (!((marsType == "cm" || marsType == "cr") && marsParam == 129 && marsDomain == "h")) {
                throw Mars2GribDeductionException(
                    "Deduction 'cluster' is only implemented for MARS type=cm/cr, param=129, domain=h.", Here());
            }
        }

        Cluster cluster;
        cluster.clusteringStartTimeStep = get_or_throw<long>(par, "clusteringStartTimeStep");
        cluster.clusteringEndTimeStep   = get_or_throw<long>(par, "clusteringEndTimeStep");
        cluster.representativeMember    = get_or_throw<long>(par, "representativeMember");
        cluster.climatologicalRegime    = get_or_throw<long>(par, "climatologicalRegime");
        cluster.clusterIdentifier       = get_or_throw<long>(mars, "number");
        cluster.NL                      = get_or_throw<long>(par, "NL");
        cluster.totalNumberOfClusters   = get_or_throw<long>(par, "totalNumberOfClusters");
        cluster.clusteringMethod        = 1;  // Root Mean Square

        // Keys associated with mars.domain=h
        cluster.northernLatitudeOfClusterDomain = 75'000000;
        cluster.southernLatitudeOfClusterDomain = 30'000000;
        cluster.easternLongitudeOfClusterDomain = 40'000000;
        cluster.westernLongitudeOfClusterDomain = 340'000000;

        cluster.numberOfForecastsInTheCluster = get_or_throw<long>(par, "numberOfForecastsInTheCluster");
        cluster.ensembleForecastNumbers       = get_or_throw<std::vector<long>>(par, "ensembleForecastNumbers");

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "`cluster` resolved from input dictionaries:";
            logMsg += " clusteringStartTimeStep=" + std::to_string(cluster.clusteringStartTimeStep);
            logMsg += ", clusteringEndTimeStep=" + std::to_string(cluster.clusteringEndTimeStep);
            logMsg += ", representativeMember=" + std::to_string(cluster.representativeMember);
            logMsg += ", climatologicalRegime=" + std::to_string(cluster.climatologicalRegime);
            logMsg += ", clusterIdentifier=" + std::to_string(cluster.clusterIdentifier);
            logMsg += ", NL='" + std::to_string(cluster.NL);
            logMsg += ", totalNumberOfClusters=" + std::to_string(cluster.totalNumberOfClusters);
            logMsg += ", clusteringMethod=" + std::to_string(cluster.clusteringMethod);
            logMsg += ", northernLatitudeOfClusterDomain=" + std::to_string(cluster.northernLatitudeOfClusterDomain);
            logMsg += ", southernLatitudeOfClusterDomain=" + std::to_string(cluster.southernLatitudeOfClusterDomain);
            logMsg += ", easternLongitudeOfClusterDomain=" + std::to_string(cluster.easternLongitudeOfClusterDomain);
            logMsg += ", westernLongitudeOfClusterDomain=" + std::to_string(cluster.westernLongitudeOfClusterDomain);
            logMsg += ", numberOfForecastsInTheCluster=" + std::to_string(cluster.numberOfForecastsInTheCluster);
            logMsg += ", ensembleForecastNumbers=[";
            for (std::size_t i = 0; i < cluster.ensembleForecastNumbers.size(); ++i) {
                logMsg += (i ? "," : "") + std::to_string(cluster.ensembleForecastNumbers[i]);
            }
            logMsg += "]";
            return logMsg;
        }());

        return cluster;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `cluster` from input dictionaries", Here()));
    };

    // Remove compiler warning
    mars2gribUnreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
