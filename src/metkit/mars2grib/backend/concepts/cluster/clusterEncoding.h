// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file clusterEncoding.h
/// @brief Implementation of the GRIB `cluster` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **cluster concept** within the mars2grib backend.
///
/// The concept is responsible for populating GRIB keys related to
/// *cluster products* (cluster mean and cluster representative) in the
/// Product Definition Section.
///
/// The concept implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `clusterApplicable`
/// - Delegation of semantic deduction to dedicated deduction functions
/// - Strict error handling with contextual concept exceptions
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of `concepts`
/// to avoid ambiguity and potential conflicts with the C++20 `concept` language
/// feature and related standard headers.
///
/// This is a deliberate design choice and must not be changed.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/cluster/clusterEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

#include "metkit/mars2grib/backend/deductions/cluster.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `cluster` concept.
///
/// This predicate determines whether the `cluster` concept is applicable
/// for a given combination of:
/// - encoding stage
/// - GRIB section
/// - concept variant
///
/// Applicability is evaluated entirely at compile time and is used by the
/// concept dispatcher to ensure that only valid concept instantiations occur.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Cluster concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
/// @note The applicability rule enables the concept only when:
/// - `Stage == StageAllocate` or `Stage == StageRuntime`
/// - `Section == SecProductDefinitionSection`
///
template <std::size_t Stage, std::size_t Section, ClusterType Variant>
constexpr bool clusterApplicable() {
    return (Section == SecProductDefinitionSection && Stage == StageRuntime) ||
           (Section == SecLocalUseSection && Stage == StageRuntime);
}

///
/// @brief Execute the `cluster` concept operation.
///
/// This function implements the runtime logic of the GRIB `cluster` concept.
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Cluster concept variant
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the GRIB output dictionary
///
/// @param[in]  mars MARS input dictionary
/// @param[in]  par  Parameter dictionary
/// @param[in]  opt  Options dictionary
/// @param[out] out  Output GRIB dictionary to be populated
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
/// If:
/// - a deduction fails
/// - a GRIB key cannot be set
///
/// @note
/// - All runtime errors are wrapped with full concept context
/// (concept name, variant, stage, section).
///
/// @see clusterApplicable
///
template <std::size_t Stage, std::size_t Section, ClusterType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void ClusterOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (clusterApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(cluster);

            const auto cluster = deductions::resolve_Cluster_or_throw(mars, par, opt);

            if constexpr (Section == SecLocalUseSection && Stage == StageRuntime) {
                set_or_throw<long>(out, "clusteringStartTimeStep", cluster.clusteringStartTimeStep);
                set_or_throw<long>(out, "clusteringEndTimeStep", cluster.clusteringEndTimeStep);
                set_or_throw<long>(out, "representativeMember", cluster.representativeMember);
                set_or_throw<long>(out, "climatologicalRegime", cluster.climatologicalRegime);
            }

            if constexpr (Section == SecProductDefinitionSection && Stage == StageRuntime) {
                set_or_throw<long>(out, "clusterIdentifier", cluster.clusterIdentifier);
                setMissing_or_throw(out, "NH");
                set_or_throw<long>(out, "NL", cluster.NL);  // clusterIdentifier of the control forecast
                set_or_throw<long>(out, "totalNumberOfClusters", cluster.totalNumberOfClusters);
                set_or_throw<long>(out, "clusteringMethod", cluster.clusteringMethod);

                set_or_throw<long>(out, "northernLatitudeOfClusterDomain", cluster.northernLatitudeOfClusterDomain);
                set_or_throw<long>(out, "southernLatitudeOfClusterDomain", cluster.southernLatitudeOfClusterDomain);
                set_or_throw<long>(out, "easternLongitudeOfClusterDomain", cluster.easternLongitudeOfClusterDomain);
                set_or_throw<long>(out, "westernLongitudeOfClusterDomain", cluster.westernLongitudeOfClusterDomain);

                setMissing_or_throw(out, "scaleFactorOfStandardDeviation");
                setMissing_or_throw(out, "scaledValueOfStandardDeviation");
                setMissing_or_throw(out, "scaleFactorOfDistanceFromEnsembleMean");
                setMissing_or_throw(out, "scaledValueOfDistanceFromEnsembleMean");

                // Key `numberOfForecastsInTheCluster` determines the size of `ensembleForecastNumbers` array
                set_or_throw<long>(out, "numberOfForecastsInTheCluster", cluster.numberOfForecastsInTheCluster);

                if (cluster.numberOfForecastsInTheCluster > 0) {
                    set_or_throw<std::vector<long>>(out, "ensembleForecastNumbers", cluster.ensembleForecastNumbers);
                }
            }
        }
        catch (...) {

            MARS2GRIB_CONCEPT_RETHROW(cluster, "Unable to set `cluster` concept...");
        }

        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(cluster, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
