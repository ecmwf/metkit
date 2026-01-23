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
 * @file RecipesCore.h
 * @brief Core data structures and DSL utilities for GRIB section recipes.
 *
 * This header defines the **fundamental types** used to describe
 * *recipes* in the mars2grib backend.
 *
 * A recipe specifies, for a given GRIB section and template number:
 * - which concepts participate in populating the section
 * - in which conceptual “mode” (or variant) each concept is applied
 *
 * Recipes are purely **declarative**. They contain no encoding logic and
 * no runtime behavior; they are consumed by higher-level orchestration
 * code to drive the concept-based encoding pipeline.
 *
 * This file provides:
 * - `ConceptSpec`: a lightweight description of a single concept usage
 * - `SectionRecipe`: a collection of concept specifications bound to a template
 * - a small constexpr DSL helper (`C`) to make recipe definitions concise
 *
 * @ingroup mars2grib_backend_sections
 */
#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace metkit::mars2grib::backend::sections::recipes {
/**
 * @brief Specification of a concept used within a section recipe.
 *
 * A `ConceptSpec` identifies:
 * - the concept name (e.g. `"level"`, `"param"`, `"statistics"`)
 * - an optional concept *type* or *mode* (e.g. `"default"`, `"analysis"`)
 *
 * The type string allows the same concept to be reused in different
 * semantic roles within different recipes or templates.
 *
 * This structure is immutable after construction and designed to be
 * cheap to copy.
 */
struct ConceptSpec {
    /// Canonical concept name
    std::string_view name;

    /// Concept type or variant identifier (defaults to `"default"`)
    std::string_view type;

    /**
     * @brief Construct a concept specification.
     *
     * @param n Concept name
     * @param t Concept type or variant (defaults to `"default"`)
     */
    constexpr ConceptSpec(std::string_view n, std::string_view t = "default") : name(n), type(t) {}
};

/**
 * @brief Declarative recipe for a GRIB section/template pair.
 *
 * A `SectionRecipe` binds a GRIB template number to an ordered list
 * of concepts that must be applied to populate the corresponding
 * section.
 *
 * The order of concepts in the vector is significant and reflects
 * the intended execution order during encoding.
 */
struct SectionRecipe {
    /// GRIB template number this recipe applies to
    uint16_t templateNumber;

    /// Ordered list of concept specifications
    std::vector<ConceptSpec> concepts;
};

/**
 * @brief Helper DSL function to construct a `ConceptSpec`.
 *
 * This function provides a compact, readable syntax for defining
 * recipes in section-specific recipe headers.
 *
 * Example usage:
 * @code
 * SectionRecipe{
 *     0,
 *     { C("param"), C("level"), C("statistics", "instant") }
 * }
 * @endcode
 *
 * @param n Concept name
 * @param t Concept type or variant (defaults to `"default"`)
 *
 * @return A constexpr `ConceptSpec` instance.
 */
constexpr ConceptSpec C(std::string_view n, std::string_view t = "default") {
    return ConceptSpec{n, t};
}

}  // namespace metkit::mars2grib::backend::sections::recipes
