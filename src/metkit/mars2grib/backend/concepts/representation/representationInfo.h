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
 * @file representationConceptInfo.h
 * @brief ConceptInfo definition for the GRIB `representation` concept.
 *
 * This header defines the **ConceptInfo** structure associated with the
 * `representation` concept in the mars2grib backend.
 *
 * A ConceptInfo acts as the **compile-time glue** between:
 * - a concept name
 * - its set of variants
 * - the applicability rules
 * - the concrete encoding operation (`RepresentationOp`)
 *
 * It is used by the concept registry machinery to:
 * - generate stage × section dispatch tables
 * - associate variants with their human-readable names
 * - enable compile-time validation and lookup
 *
 * This file contains **no runtime state** and **no encoding logic**.
 * All behavior is resolved at compile time.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <cstdint>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/representation/representationEncoding.h"
#include "metkit/mars2grib/backend/concepts/representation/representationEnum.h"


namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time metadata and entry-point provider for the `representation` concept.
 *
 * `RepresentationConceptInfo` exposes the minimal interface required by the
 * mars2grib concept registry:
 *
 * - a canonical concept name
 * - a compile-time dispatcher (`entry`) that resolves to the correct
 *   encoding operation for a given stage, section, and variant
 * - a mapping from variant identifiers to string names
 *
 * The registry uses this structure to generate a
 * **[Stage × Section] function table** for each supported
 * `RepresentationType` variant.
 *
 * @note
 * This structure is intentionally stateless and fully constexpr-friendly.
 */
struct RepresentationConceptInfo {

    /**
     * @brief Canonical name of the `representation` concept.
     *
     * This identifier is used as the primary key in the concept registry
     * and must match the name exposed by the corresponding enum header.
     */
    static constexpr const char* name = representationName.data();

    /**
     * @brief Resolve the encoding entry point for a specific stage, section, and variant.
     *
     * This function is evaluated entirely at compile time and returns:
     * - a pointer to the appropriate `RepresentationOp` specialization if the concept
     *   is applicable for the given parameters
     * - `nullptr` otherwise
     *
     * The returned function pointer is stored in the concept dispatch table
     * and invoked at runtime by the encoder.
     *
     * @tparam Stage        Encoding stage (allocate, preset, runtime)
     * @tparam Section      GRIB section index
     * @tparam Variant      Representation concept variant
     * @tparam MarsDict_t   Type of the MARS dictionary
     * @tparam ParDict_t    Type of the parameter dictionary
     * @tparam OptDict_t    Type of the options dictionary
     * @tparam OutDict_t    Type of the output GRIB dictionary
     *
     * @return Function pointer to the corresponding `RepresentationOp`,
     *         or `nullptr` if the concept is not applicable.
     *
     * @note
     * Applicability is determined by
     * `representationApplicable<Stage, Section, Variant>()`.
     */
    template <std::size_t Stage, std::size_t Section, RepresentationType Variant, class MarsDict_t, class ParDict_t,
              class OptDict_t, class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (representationApplicable<Stage, Section, Variant>()) {
            return &RepresentationOp<Stage, Section, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    /**
     * @brief Retrieve the human-readable name of a `representation` variant.
     *
     * This function maps a compile-time variant identifier to its
     * canonical string representation.
     *
     * It is used by:
     * - the concept registry
     * - logging and debugging utilities
     * - diagnostic and error reporting
     *
     * @tparam Variant Representation variant identifier
     * @return String view representing the variant name
     */
    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(representationTypeName<static_cast<RepresentationType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::concepts_
