/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file representationOp.h
 * @brief Implementation of the GRIB `representation` concept operation.
 *
 * This header defines the applicability rules and execution logic for the
 * **representation concept**, which is responsible for configuring the
 * GRIB *Grid Definition Section (Section 3)*.
 *
 * The representation concept controls how the spatial grid of a field
 * is described in GRIB, including:
 * - grid type selection (lat/lon, Gaussian, spectral, HEALPix, …)
 * - grid topology and resolution parameters
 * - allocation and population of grid-related metadata
 *
 * The concept operates in multiple encoding stages and performs
 * **variant-specific logic** depending on the selected
 * `RepresentationType`.
 *
 * ---
 *
 * ## Encoding stages
 *
 * The concept is active in the following stages:
 *
 * - **StageAllocate**
 *   - Selection of the GRIB grid type
 *   - Structural validation against the Grid Definition Template
 *   - Allocation of representation-specific data (e.g. PL arrays)
 *
 * - **StagePreset**
 *   - Population of grid geometry parameters
 *   - Encoding of resolution, truncation, and coordinate metadata
 *
 * The exact operations performed depend on the selected representation
 * variant.
 *
 * ---
 *
 * ## Supported representation variants
 *
 * The following `RepresentationType` variants are currently handled:
 *
 * - `Latlon`
 * - `RegularGaussian`
 * - `ReducedGaussian`
 * - `SphericalHarmonics`
 * - `Healpix`
 *
 * The following variants are recognized but **not implemented**:
 *
 * - `Orca`
 * - `Fesom`
 *
 * Attempting to use unsupported variants results in a concept-level
 * exception.
 *
 * ---
 *
 * ## Geometry handling
 *
 * Geometry parameters are currently retrieved **directly** from the
 * geometry dictionary (`geo`) using `get_or_throw`.
 *
 * @warning
 * This is a transitional design.
 *
 * A dedicated grid/geometry deduction layer does not exist yet.
 * As a consequence:
 * - The concept performs direct access to geometry keys
 * - Validation of geometry consistency is minimal
 * - Responsibilities between geometry handling and encoding are not
 *   fully separated
 *
 * This will be refactored in a future iteration.
 *
 * ---
 *
 * ## Applicability model
 *
 * All representation variants are considered applicable in:
 *
 * - `StageAllocate`
 * - `StagePreset`
 *
 * for `SecGridDefinitionSection`.
 *
 * Variant-specific behavior is implemented entirely inside the
 * operation body using `if constexpr`.
 *
 * ---
 *
 * ## Error handling
 *
 * - Structural mismatches with GRIB templates are detected via
 *   validation helpers.
 * - Unsupported or unknown representation variants result in
 *   concept-level exceptions.
 * - All failures are rethrown with full concept context.
 *
 * ---
 *
 * @note
 * The namespace name `concepts_` is intentionally used instead of
 * `concepts` to avoid conflicts with the C++20 `concept` language
 * feature.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/representation/representationEnum.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchGridDefinitionTemplateNumber.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `representation` concept.
 *
 * This predicate determines whether the `representation` concept is
 * instantiated for a given encoding stage and GRIB section.
 *
 * All representation variants are applicable during:
 * - `StageAllocate`
 * - `StagePreset`
 *
 * when operating on the *Grid Definition Section*.
 *
 * Variant-specific behavior is handled inside the concept operation.
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Representation variant
 *
 * @return `true` if the concept is applicable, `false` otherwise.
 */
template <std::size_t Stage, std::size_t Section, RepresentationType Variant>
constexpr bool representationApplicable() {

    bool condition1 = Stage == StageAllocate && Section == SecGridDefinitionSection;
    bool condition2 = Stage == StagePreset && Section == SecGridDefinitionSection;

    return condition1 || condition2;
}


/**
 * @brief Execute the `representation` concept operation.
 *
 * This function implements the runtime logic of the GRIB `representation`
 * concept.
 *
 * Depending on the encoding stage and the selected `RepresentationType`,
 * it performs:
 *
 * - validation of the Grid Definition Template
 * - selection of the GRIB grid type
 * - extraction of geometry parameters
 * - encoding of grid topology and resolution metadata
 *
 * The logic is entirely **variant-specific** and selected at compile time
 * using `if constexpr`.
 *
 * @tparam Stage      Encoding stage
 * @tparam Section    GRIB section index
 * @tparam Variant    Representation variant
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam GeoDict_t  Type of the geometry dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @param[in]  mars MARS input dictionary (currently unused)
 * @param[in]  geo  Geometry dictionary providing grid parameters
 * @param[in]  par  Parameter dictionary (currently unused)
 * @param[in]  opt  Options dictionary
 * @param[out] out  Output GRIB dictionary to be populated
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
 *         If:
 *         - the concept is called when not applicable
 *         - the GRIB Grid Definition Template does not match expectations
 *         - required geometry information is missing or inconsistent
 *         - the representation variant is unsupported
 *
 * @note
 * This concept currently performs direct access to geometry parameters.
 * A dedicated grid deduction layer will be introduced in the future.
 */
template <std::size_t Stage, std::size_t Section, RepresentationType Variant, class MarsDict_t, class GeoDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
void RepresentationOp(const MarsDict_t& mars, const GeoDict_t& geo, const ParDict_t& par, const OptDict_t& opt,
                      OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (representationApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(representation);

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Stage == StageAllocate) {

                if constexpr (Variant == RepresentationType::Latlon) {

                    // Checks/Validation
                    validation::match_GridDefinitionTemplateNumber_or_throw(opt, out, {0});

                    // Encoding
                    set_or_throw<std::string>(out, "gridType", "regular_ll");
                }
                else if constexpr (Variant == RepresentationType::RegularGaussian) {

                    // Checks/Validation
                    validation::match_GridDefinitionTemplateNumber_or_throw(opt, out, {40});

                    // Encoding
                    set_or_throw<std::string>(out, "gridType", "regular_gg");
                }
                else if constexpr (Variant == RepresentationType::ReducedGaussian) {

                    // Checks/Validation
                    validation::match_GridDefinitionTemplateNumber_or_throw(opt, out, {40});

                    // Deductions
                    std::vector<long> PlArray = get_or_throw<std::vector<long>>(geo, "pl");

                    long numberOfParallelsBetweenAPoleAndTheEquator =
                        get_or_throw<long>(geo, "numberOfParallelsBetweenAPoleAndTheEquator");

                    // Encoding
                    set_or_throw<std::string>(out, "gridType", "reduced_gg");
                    set_or_throw<long>(out, "interpretationOfNumberOfPoints", 1L);
                    set_or_throw<long>(out, "numberOfParallelsBetweenAPoleAndTheEquator",
                                       numberOfParallelsBetweenAPoleAndTheEquator);
                    set_or_throw<std::vector<long>>(out, "pl", PlArray);
                }
                else if constexpr (Variant == RepresentationType::SphericalHarmonics) {

                    // Checks/Validation
                    validation::match_GridDefinitionTemplateNumber_or_throw(opt, out, {50});

                    // Encoding
                    set_or_throw<std::string>(out, "gridType", "sh");
                }
                else if constexpr (Variant == RepresentationType::Healpix) {

                    // Checks/Validation
                    validation::match_GridDefinitionTemplateNumber_or_throw(opt, out, {150});

                    // Encoding
                    set_or_throw<std::string>(out, "gridType", "healpix");
                }
                else if constexpr (Variant == RepresentationType::Orca) {

                    // Checks/Validation
                    validation::match_GridDefinitionTemplateNumber_or_throw(opt, out, {101});

                    // Not implemented error
                    MARS2GRIB_CONCEPT_THROW(representation, "Support for Orca representation not implemented...");
                }
                else if constexpr (Variant == RepresentationType::Fesom) {

                    // Checks
                    validation::match_GridDefinitionTemplateNumber_or_throw(opt, out, {101});

                    // Not implemented error
                    MARS2GRIB_CONCEPT_THROW(representation, "Support for Fesom representation not implemented...");
                }
                else {
                    MARS2GRIB_CONCEPT_THROW(representation, "Unknown `representation` variant...");
                }
            }


            // Preset data representation
            if constexpr (Stage == StagePreset) {
                if constexpr (Variant == RepresentationType::Latlon) {

                    // Deductions
                    long Ni = get_or_throw<long>(geo, "numberOfPointsAlongAParallel");
                    long Nj = get_or_throw<long>(geo, "numberOfPointsAlongAMeridian");
                    const auto latitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfFirstGridPointInDegrees");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");
                    const auto latitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfLastGridPointInDegrees");
                    const auto longitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfLastGridPointInDegrees");
                    const auto iDirectionIncrementInDegrees = get_or_throw<double>(geo, "iDirectionIncrementInDegrees");
                    const auto jDirectionIncrementInDegrees = get_or_throw<double>(geo, "jDirectionIncrementInDegrees");

                    // Encoding
                    set_or_throw<long>(out, "Ni", Ni);
                    set_or_throw<long>(out, "Nj", Nj);
                    set_or_throw(out, "latitudeOfFirstGridPointInDegrees", latitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "latitudeOfLastGridPointInDegrees", latitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "longitudeOfLastGridPointInDegrees", longitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "iDirectionIncrementInDegrees", iDirectionIncrementInDegrees);
                    set_or_throw(out, "jDirectionIncrementInDegrees", jDirectionIncrementInDegrees);
                }
                else if constexpr (Variant == RepresentationType::RegularGaussian) {

                    // Deductions
                    const auto truncateDegrees = get_opt<long>(geo, "truncateDegrees").value_or(0);
                    // long numberOfPointsAlongAMeridian = get_or_throw<long>( geo, "numberOfPointsAlongAMeridian" ); //
                    // TODO (knobel) long numberOfPointsAlongAParallel = get_or_throw<long>( geo,
                    // "numberOfPointsAlongAParallel" );  // TODO (knobel)
                    const auto latitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfFirstGridPointInDegrees");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");
                    const auto latitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfLastGridPointInDegrees");
                    const auto longitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfLastGridPointInDegrees");
                    long numberOfParallelsBetweenAPoleAndTheEquator =
                        get_or_throw<long>(geo, "numberOfParallelsBetweenAPoleAndTheEquator");
                    const auto iDirectionIncrementInDegrees = get_or_throw<double>(geo, "iDirectionIncrementInDegrees");

                    // Encoding
                    set_or_throw(out, "truncateDegrees", truncateDegrees);
                    // set_or_throw<long>( out, "numberOfPointsAlongAMeridian", numberOfPointsAlongAMeridian );  // TODO
                    // (knobel) set_or_throw<long>( out, "numberOfPointsAlongAParallel", numberOfPointsAlongAParallel );
                    // // TODO (knobel)
                    set_or_throw(out, "latitudeOfFirstGridPointInDegrees", latitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "latitudeOfLastGridPointInDegrees", latitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "longitudeOfLastGridPointInDegrees", longitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "iDirectionIncrementInDegrees", iDirectionIncrementInDegrees);
                }
                else if constexpr (Variant == RepresentationType::ReducedGaussian) {

                    // Deductions
                    const auto truncateDegrees = get_opt<long>(geo, "truncateDegrees").value_or(0);
                    // long numberOfPointsAlongAMeridian = get_or_throw<long>( geo, "numberOfPointsAlongAMeridian" ); //
                    // TODO (knobel)
                    const auto latitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfFirstGridPointInDegrees");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");
                    const auto latitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "latitudeOfLastGridPointInDegrees");
                    const auto longitudeOfLastGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfLastGridPointInDegrees");
                    // long numberOfParallelsBetweenAPoleAndTheEquator = get_or_throw<long>( geo,
                    // "numberOfParallelsBetweenAPoleAndTheEquator" );

                    // Encoding
                    set_or_throw<long>(out, "truncateDegrees", truncateDegrees);
                    // set_or_throw<long>( out, "numberOfPointsAlongAMeridian", numberOfPointsAlongAMeridian );  // TODO
                    // (knobel)
                    set_or_throw(out, "latitudeOfFirstGridPointInDegrees", latitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                    set_or_throw(out, "latitudeOfLastGridPointInDegrees", latitudeOfLastGridPointInDegrees);
                    set_or_throw(out, "longitudeOfLastGridPointInDegrees", longitudeOfLastGridPointInDegrees);
                    // set_or_throw<long>( out, "numberOfParallelsBetweenAPoleAndTheEquator",
                    // numberOfParallelsBetweenAPoleAndTheEquator );
                    setMissing_or_throw(out, "iDirectionIncrement");
                }
                else if constexpr (Variant == RepresentationType::SphericalHarmonics) {

                    // Deductions
                    long pentagonalResolutionParameterJ = get_or_throw<long>(geo, "pentagonalResolutionParameterJ");
                    long pentagonalResolutionParameterK = get_or_throw<long>(geo, "pentagonalResolutionParameterK");
                    long pentagonalResolutionParameterM = get_or_throw<long>(geo, "pentagonalResolutionParameterM");

                    // Encoding
                    set_or_throw<long>(out, "pentagonalResolutionParameterJ", pentagonalResolutionParameterJ);
                    set_or_throw<long>(out, "pentagonalResolutionParameterK", pentagonalResolutionParameterK);
                    set_or_throw<long>(out, "pentagonalResolutionParameterM", pentagonalResolutionParameterM);
                }
                else if constexpr (Variant == RepresentationType::Healpix) {

                    // Deductions
                    long nside              = get_or_throw<long>(geo, "nside");
                    long orderingConvention = get_or_throw<long>(geo, "orderingConvention");
                    const auto longitudeOfFirstGridPointInDegrees =
                        get_or_throw<double>(geo, "longitudeOfFirstGridPointInDegrees");

                    // Encoding
                    set_or_throw<long>(out, "nside", nside);
                    set_or_throw<long>(out, "orderingConvention", orderingConvention);
                    set_or_throw(out, "longitudeOfFirstGridPointInDegrees", longitudeOfFirstGridPointInDegrees);
                }
                else if constexpr (Variant == RepresentationType::Orca) {
                    MARS2GRIB_CONCEPT_THROW(representation, "Support for Orca representation not implemented...");
                }
                else if constexpr (Variant == RepresentationType::Fesom) {
                    MARS2GRIB_CONCEPT_THROW(representation, "Support for Fesom representation not implemented...");
                }
                else {
                    MARS2GRIB_CONCEPT_THROW(representation, "Unknown `representation` variant...");
                };
            };
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(representation, "Unable to set `representation` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(representation, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
