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
/// @file Recipe.h
/// @brief Runtime representation of a fully expanded section recipe.
///
/// This header defines `Recipe`, an **immutable runtime object** produced from
/// the compile-time section-recipe DSL.
///
/// A `Recipe` encapsulates:
/// - A GRIB **template number**
/// - A multidimensional selection space derived from `Select<>` grammar nodes
/// - The total number of **valid variant combinations**
///
/// Conceptually, a recipe represents the *Cartesian product* of a sequence of
/// concept-variant selectors. Each point in this space corresponds to a
/// concrete encoding configuration and can be materialized on demand as a
/// `ResolvedTemplateData` instance.
///
/// The class is designed for:
/// - Fast lookup
/// - Predictable iteration
/// - Deterministic decoding via mixed-radix arithmetic
///
/// Instances are immutable after construction and are intended to be stored
/// and traversed in hot paths during encoding plan construction.
///
/// Debug and introspection facilities are provided for diagnostics only and
/// are not part of the performance-critical API.
///
/// @ingroup mars2grib_backend_section_recipes
///
#pragma once

// System includes
#include <array>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ResolvedTemplateData.h"
#include "metkit/mars2grib/backend/sections/resolver/Select.h"


namespace metkit::mars2grib::backend::sections::resolver::dsl {

///
/// @brief Runtime, immutable container defining a GRIB section template number.
///
/// A `Recipe` represents the **runtime realization of a GRIB section template
/// definition**.
///
/// In the GRIB model, a *template number* is not defined by a single choice,
/// but by an **ordered set of concepts** contributing to the same section.
/// Each concept may participate in the definition of the template using
/// **different variants**, and different combinations of variants may map
/// to the same template number.
///
/// As a consequence, the process of defining a template number is
/// **inherently combinatorial**.
///
/// The role of a `Recipe` is to:
/// - Bind a specific GRIB template number
/// - Describe the complete space of valid concept-variant combinations
/// that realize that template
///
/// The valid variants for each concept are expressed at compile time using
/// the `Select<>` DSL object, which specifies:
/// - Which concept participates
/// - Which variants of that concept are admissible for the template
///
/// At runtime, the recipe materializes this information as a multidimensional
/// selection space, where:
/// - Each dimension corresponds to one concept
/// - Each dimension contains the list of allowed global variant identifiers
///
/// Individual encoding configurations are obtained by enumerating this space
/// using mixed-radix decoding, preserving the original concept order.
///
/// The class is intentionally opaque and immutable:
/// - No mutation after construction
/// - No exposure of internal storage
/// - Construction only via the `make_recipe` factory
///
/// This design ensures deterministic behavior, efficient lookup, and
/// suitability for hot-path execution during encoding plan construction.
///
class Recipe {
public:

    ///
    /// @brief Return the total number of valid variant combinations.
    ///
    std::size_t numberOfCombinations() const noexcept { return nCombinations_; }

    ///
    /// @brief Materialize a resolved recipe entry.
    ///
    /// This function decodes the given linear index into a concrete
    /// combination of concept variants and returns it as a
    /// `ResolvedTemplateData` payload.
    ///
    /// Mixed-radix decoding is used, preserving the original selector order.
    /// The rightmost selector varies fastest.
    ///
    /// @param[in] i Linear combination index
    ///
    /// @return Fully populated resolved template payload
    ///
    /// @throws std::out_of_range
    /// If @p i is greater than or equal to the number of combinations
    ///
    ResolvedTemplateData getEntry(std::size_t i) const {

        if (i >= nCombinations_) {
            throw std::out_of_range("Recipe::getEntry index out of range");
        }

        ResolvedTemplateData entry;
        entry.templateNumber = templateNumber_;
        entry.count          = variants_.size();

        std::size_t remainder = i;

        // Mixed-radix decoding:
        // selector order preserved
        // rightmost selector varies fastest
        for (std::size_t d = variants_.size(); d-- > 0;) {
            const std::size_t radix = sizes_[d];
            const std::size_t idx   = remainder % radix;
            remainder /= radix;

            entry.variantIndices[d] = variants_[d][idx];
        }

        return entry;
    }

    ///
    /// @brief Print a human-readable description of the recipe.
    ///
    /// @param[in]  prefix Line prefix used for indentation
    /// @param[out] os     Output stream
    ///
    void debug_print(const std::string& prefix, std::ostream& os) const {

        using metkit::mars2grib::backend::concepts_::GeneralRegistry;

        os << prefix << " :: Recipe\n";
        os << prefix << " ::   templateNumber    : " << templateNumber_ << std::endl;
        os << prefix << " ::   dimensions        : " << variants_.size() << std::endl;
        os << prefix << " ::   nCombinations     : " << nCombinations_ << std::endl;

        for (std::size_t d = 0; d < variants_.size(); ++d) {
            os << prefix << " ::  dimension[" << d << "]" << std::endl;
            os << prefix << " ::    radix           : " << sizes_[d] << std::endl;

            os << prefix << " ::    variants glbId  : [ ";
            const auto& dim = variants_[d];
            for (std::size_t i = 0; i < dim.size(); ++i) {
                os << dim[i];
                if (i + 1 < dim.size())
                    os << ", ";
            }
            os << " ] " << std::endl;

            os << prefix << " ::    variants names  : [ ";
            for (std::size_t i = 0; i < dim.size(); ++i) {
                std::size_t id    = dim[i];
                std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
                std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
                os << "\"" << cname << "::" << vname << "\"";
                if (i + 1 < dim.size())
                    os << ", ";
            }
            os << " ]" << std::endl;
        }
    }

    ///
    /// @brief Convert the recipe to a JSON-like string.
    ///
    /// Intended exclusively for diagnostics and debugging.
    ///
    /// @return JSON-style string representation
    ///
    std::string debug_to_json() const {

        using metkit::mars2grib::backend::concepts_::GeneralRegistry;

        std::ostringstream oss;

        oss << "{ \"Recipe\":{";
        oss << "\"templateNumber\":" << templateNumber_ << ", ";
        oss << "\"dimensions\":" << variants_.size() << ", ";
        oss << "\"nCombinations\":" << nCombinations_ << ", ";
        oss << "\"selectors\":[";
        for (std::size_t d = 0; d < variants_.size(); ++d) {
            oss << "{\"dimension\":" << d << ", ";
            oss << "\"radix\":" << sizes_[d] << ", ";
            oss << "\"variantIndices\": [";
            const auto& dim = variants_[d];
            for (std::size_t i = 0; i < dim.size(); ++i) {
                oss << dim[i];
                if (i + 1 < dim.size())
                    oss << ", ";
            }
            oss << "], " << std::endl;

            oss << "\"variantNames\": [";
            for (std::size_t i = 0; i < dim.size(); ++i) {
                std::size_t id    = dim[i];
                std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
                std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
                oss << "\"" << cname << "::" << vname << "\"";
                if (i + 1 < dim.size())
                    oss << ", ";
            }
            oss << " ]}" << std::endl;
            if (d + 1 < variants_.size())
                oss << ", ";
        }
        oss << "]}}";

        return oss.str();
    }


private:

    using Dimension  = std::vector<std::size_t>;
    using Dimensions = std::vector<Dimension>;
    using Sizes      = std::vector<std::size_t>;

    const std::size_t templateNumber_;
    const Dimensions variants_;
    const Sizes sizes_;
    const std::size_t nCombinations_;

private:

    Recipe(std::size_t tpl, Dimensions&& variants, Sizes&& sizes, std::size_t nComb) :
        templateNumber_(tpl), variants_(std::move(variants)), sizes_(std::move(sizes)), nCombinations_(nComb) {}

    template <std::size_t Tpl, class... Selects>
    friend Recipe make_recipe();
};


///
/// @brief Factory function converting DSL grammar to a runtime recipe.
///
/// This function erases the compile-time `Select<>` grammar and produces
/// a fully materialized `Recipe` object suitable for runtime use.
///
/// @tparam TemplateNumber GRIB template number
/// @tparam Selects        Sequence of selector grammar nodes
///
/// @return Immutable runtime recipe instance
///
template <std::size_t TemplateNumber, class... Selects>
inline Recipe make_recipe() {

    constexpr std::size_t N = sizeof...(Selects);

    Recipe::Dimensions variants;
    Recipe::Sizes sizes;

    variants.reserve(N);
    sizes.reserve(N);

    // Erase Select<> grammar here
    (
        [&] {
            variants.emplace_back(Selects::ids.begin(), Selects::ids.end());
            sizes.push_back(Selects::ids.size());
        }(),
        ...);

    std::size_t nComb = 1;
    for (std::size_t s : sizes) {
        nComb *= s;
    }

    return Recipe{TemplateNumber, std::move(variants), std::move(sizes), nComb};
}

}  // namespace metkit::mars2grib::backend::sections::resolver::dsl
