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
 * @file marsConceptInfo.h
 * @brief ConceptInfo definition for the GRIB `mars` concept.
 *
 * This header defines the **ConceptInfo** structure associated with the
 * `mars` concept in the mars2grib backend.
 *
 * A ConceptInfo acts as the **compile-time glue** between:
 * - a concept name
 * - its set of variants
 * - the applicability rules
 * - the concrete encoding operation (`MarsOp`)
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
#include "metkit/mars2grib/backend/concepts/mars/marsEncoding.h"
#include "metkit/mars2grib/backend/concepts/mars/marsEnum.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time metadata and entry-point provider for the `mars` concept.
 *
 * `MarsConceptInfo` exposes the minimal interface required by the
 * mars2grib concept registry:
 *
 * - a canonical concept name
 * - a compile-time dispatcher (`entry`) that resolves to the correct
 *   encoding operation for a given stage, section, and variant
 * - a mapping from variant identifiers to string names
 *
 * The registry uses this structure to generate a
 * **[Stage × Section] function table** for each supported `MarsType` variant.
 *
 * @note
 * This structure is intentionally stateless and fully constexpr-friendly.
 */
struct MarsConceptInfo {

    /**
     * @brief Canonical name of the `mars` concept.
     *
     * This identifier is used as the primary key in the concept registry
     * and must match the name exposed by the corresponding enum header.
     */
    static constexpr const char* name = marsName.data();

    /**
     * @brief Resolve the encoding entry point for a specific stage, section, and variant.
     *
     * This function is evaluated entirely at compile time and returns:
     * - a pointer to the appropriate `MarsOp` specialization if the concept
     *   is applicable for the given parameters
     * - `nullptr` otherwise
     *
     * The returned function pointer is stored in the concept dispatch table
     * and invoked at runtime by the encoder.
     *
     * @tparam Stage      Encoding stage (allocate, preset, runtime)
     * @tparam Sec        GRIB section index
     * @tparam Variant    Mars concept variant
     * @tparam MarsDict_t Type of the MARS dictionary
     * @tparam ParDict_t  Type of the parameter dictionary
     * @tparam OptDict_t  Type of the options dictionary
     * @tparam OutDict_t  Type of the output GRIB dictionary
     *
     * @return Function pointer to the corresponding `MarsOp`, or `nullptr`
     *         if the concept is not applicable.
     *
     * @note
     * Applicability is determined by `marsApplicable<Stage, Sec, Variant>()`.
     */
    template <std::size_t Stage, std::size_t Sec, MarsType Variant, class MarsDict_t, class ParDict_t, class OptDict_t,
              class OutDict_t>
    static constexpr Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> entry() {
        if constexpr (marsApplicable<Stage, Sec, Variant>()) {
            return &MarsOp<Stage, Sec, Variant, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
        }
        else {
            return nullptr;
        }

        // Avoid compile warnings
        __builtin_unreachable();
    }

    /**
     * @brief Retrieve the human-readable name of a `mars` variant.
     *
     * This function maps a compile-time variant identifier to its
     * canonical string representation.
     *
     * It is used by:
     * - the concept registry
     * - logging and debugging utilities
     * - diagnostic and error reporting
     *
     * @tparam Variant Mars variant identifier
     * @return String view representing the variant name
     */
    template <auto Variant>
    static std::string_view variantName() {
        return std::string_view(marsTypeName<static_cast<MarsType>(Variant)>());
    }
};

}  // namespace metkit::mars2grib::backend::concepts_
