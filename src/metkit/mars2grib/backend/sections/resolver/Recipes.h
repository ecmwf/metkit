/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file Recipes.h
/// @brief Runtime container for all template recipes of a GRIB section.
///
/// This header defines `Recipes`, a **section-scoped runtime container**
/// representing the complete set of GRIB template definitions applicable
/// to a single GRIB section.
///
/// A `Recipes` object is defined **per section** and aggregates all
/// `Recipe` instances contributing to that section. Each `Recipe`
/// defines one GRIB template number together with the full combinatorial
/// space of concept-variant combinations that realize that template.
///
/// Conceptually:
/// - A GRIB section may admit multiple template numbers
/// - Each template number is defined by an ordered set of concepts
/// - Each concept may participate with multiple admissible variants
///
/// The role of `Recipes` is to:
/// - Collect all template definitions for a given section
/// - Preserve their ordering
/// - Provide a uniform way to expand them into a flat list of
/// `ResolvedTemplateData` payloads
///
/// The container is immutable after construction and is designed to be
/// traversed in hot paths during section resolution and encoding plan
/// construction.
///
/// @ingroup mars2grib_backend_section_recipes
///
#pragma once

// System includes
#include <cstddef>
#include <vector>

// Project includes
#include "metkit/mars2grib/backend/sections/resolver/Recipe.h"
#include "metkit/mars2grib/backend/sections/resolver/ResolvedTemplateData.h"

namespace metkit::mars2grib::backend::sections::resolver::dsl {

///
/// @brief Runtime, immutable container for all recipes of a single GRIB section.
///
/// A `Recipes` instance represents the **complete template-definition space**
/// for one specific GRIB section.
///
/// It owns:
/// - The section identifier
/// - An ordered list of `Recipe` objects, each corresponding to one
/// GRIB template number valid for that section
///
/// The container provides a single expansion operation that materializes
/// all possible resolved templates (`ResolvedTemplateData`) for the section,
/// by expanding each recipe and concatenating their combinatorial spaces.
///
/// No mutation or filtering is performed at this level.
///
class Recipes {
public:

    ///
    /// @brief Construct a section-scoped recipe container.
    ///
    /// @param[in] sectionId GRIB section identifier
    /// @param[in] recipes   Ordered list of recipe definitions for the section
    ///
    Recipes(std::size_t sectionId, std::vector<const Recipe*>&& recipes) :
        sectionId_(sectionId), recipes_(std::move(recipes)) {}

    ///
    /// @brief Return the GRIB section identifier.
    ///
    std::size_t sectionId() const noexcept { return sectionId_; }

    ///
    /// @brief Expand all recipes into resolved template payloads.
    ///
    /// This function materializes the full set of resolved templates
    /// defined for the section by:
    /// - Iterating over all recipes
    /// - Expanding each recipe’s combinatorial space
    /// - Concatenating the results in recipe order
    ///
    /// @return Vector of resolved template payloads
    ///
    std::vector<ResolvedTemplateData> getPayload() const {

        // ---- compute total size ----
        std::size_t total = 0;
        for (const Recipe* r : recipes_) {
            total += r->numberOfCombinations();
        }

        // ---- allocate payload ----
        std::vector<ResolvedTemplateData> payload;
        payload.reserve(total);

        // ---- expand recipes in order ----
        for (const Recipe* r : recipes_) {
            const std::size_t n = r->numberOfCombinations();
            for (std::size_t i = 0; i < n; ++i) {
                payload.push_back(r->getEntry(i));
            }
        }

        return payload;
    }

    ///
    /// @brief Print a human-readable description of the section recipes.
    ///
    /// This function prepends the section identifier and delegates the
    /// detailed output to the nested `Recipe` objects.
    ///
    /// @param[in]  prefix Line prefix used for indentation
    /// @param[out] os     Output stream
    ///
    void debug_print(const std::string& prefix, std::ostream& os) const {

        os << prefix << " :: Recipes\n";
        os << prefix << " ::   sectionId : " << sectionId_ << "\n";
        os << prefix << " ::   nRecipes  : " << recipes_.size() << "\n";

        for (std::size_t i = 0; i < recipes_.size(); ++i) {
            os << prefix << " ::   recipe[" << i << "]\n";
            recipes_[i]->debug_print(prefix + std::string(" ::   "), os);
        }
    }

    ///
    /// @brief Convert the section recipes to a JSON-like string.
    ///
    /// The output includes the section identifier and the JSON
    /// representation of each nested recipe.
    ///
    /// @return JSON-style string representation
    ///
    std::string debug_to_json() const {

        std::ostringstream oss;

        oss << "{ \"Recipes\": { "
            << "\"sectionId\": " << sectionId_ << ", "
            << "\"recipes\": [ ";

        for (std::size_t i = 0; i < recipes_.size(); ++i) {
            oss << recipes_[i]->debug_to_json();
            if (i + 1 < recipes_.size()) {
                oss << ", ";
            }
        }

        oss << " ] } }";

        return oss.str();
    }

private:

    const std::size_t sectionId_;
    const std::vector<const Recipe*> recipes_;
};

}  // namespace metkit::mars2grib::backend::sections::resolver::dsl
