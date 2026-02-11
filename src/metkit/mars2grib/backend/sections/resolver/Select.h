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
/// @file Select.h
/// @brief Compile-time selector for concept variants in section recipe definitions.
///
/// This header defines the `Select` template, a **compile-time DSL building block**
/// used by section recipes to specify *which variants of a given concept* are
/// applicable for a recipe entry.
///
/// The selector supports two usage modes:
/// - **Explicit selection** of a fixed set of concept variants
/// - **Wildcard selection** (`any`), meaning *all variants* of the concept
///
/// The selection is resolved entirely at compile time and materialized as a
/// constant array of *global variant identifiers*, as defined by the
/// `GeneralRegistry`.
///
/// This mechanism allows section recipes to:
/// - Remain declarative and concise
/// - Avoid runtime branching or lookups
/// - Integrate seamlessly with the compile-time recipe expansion pipeline
///
/// @note
/// This type is part of the *section recipe DSL* and is not intended to be used
/// directly by runtime encoding logic.
///
/// @ingroup mars2grib_backend_section_recipes
///
#pragma once

// System includes
#include <array>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <typeinfo>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::resolver::dsl {

///
/// @file Select.h
/// @brief Compile-time selector defining admissible variants of a concept.
///
/// This header defines `Select`, a **compile-time DSL primitive** used to
/// express *which variants of a given concept* are admissible when defining
/// a GRIB section template.
///
/// A `Select` always refers to **exactly one concept** and defines a
/// **subset of its variants**. This subset represents the variants of the
/// concept that are allowed to participate in the definition of a specific
/// template number.
///
/// The selector supports two explicit modes:
///
/// - **Explicit selection**
/// When one or more variant tags are provided, only those variants are
/// considered valid for the concept.
///
/// - **Implicit full selection (wildcard)**
/// When no variants are provided, *all variants* of the concept are
/// implicitly selected.
///
/// In other words:
/// - `Select<Concept, V1, V2>` selects **only** variants `V1` and `V2`
/// - `Select<Concept>` selects **all** variants of `Concept`
///
/// `Select` objects are the fundamental building blocks used by every
/// `Recipe` to define the **rules governing template-number selection**.
///
/// A recipe is defined by an ordered list of `Select` objects, one per
/// participating concept. Together, these selectors describe the full
/// combinatorial space of admissible concept-variant combinations that
/// realize a given GRIB template number.
///
/// The selection is resolved entirely at compile time and materialized
/// as a constant list of global variant identifiers, ensuring:
/// - Zero runtime overhead
/// - Deterministic behavior
/// - Early validation of recipe definitions
///
/// @note
/// `Select` does not perform any runtime logic. It is a declarative,
/// compile-time construct whose sole responsibility is to describe
/// admissible variant subsets for a concept.
///
/// @see Recipe
/// @ingroup mars2grib_backend_section_resolver
///
template <typename Concept, auto... Vs>
struct Select {

    /// Concept associated with this selector
    using concept_type = Concept;

    /// True if the selector matches all variants of the concept
    static constexpr bool is_any = (sizeof...(Vs) == 0);

    /// Number of selected variants
    static constexpr std::size_t count = is_any ? Concept::variant_count : sizeof...(Vs);

private:

    ///
    /// @brief Generate the array of global variant identifiers.
    ///
    /// This helper resolves the selection rule to a concrete array of
    /// variant IDs using the `GeneralRegistry`.
    ///
    /// The resolution is performed entirely at compile time.
    ///
    /// @return `std::array<std::size_t, count>` containing global variant IDs
    ///
    static constexpr auto generate_ids() {

        using metkit::mars2grib::backend::concepts_::GeneralRegistry;

        if constexpr (is_any) {
            return GeneralRegistry::make_id_array_from_concept<Concept>();
        }
        else {
            return GeneralRegistry::make_id_array_from_variants<Concept, Vs...>();
        }

        // Remove compiler warning
        __builtin_unreachable();
    }

public:

    /// Compile-time array of selected global variant identifiers
    inline static constexpr auto ids = generate_ids();

    ///
    /// @brief Print a human-readable description of the selector.
    ///
    /// This function emits a structured textual representation of the
    /// selector configuration, including:
    /// - Concept type
    /// - Wildcard status
    /// - Number of selected variants
    /// - Variant indices
    /// - Fully-qualified variant names
    ///
    /// @param[in] prefix Prefix string prepended to each output line
    /// @param[in,out] os Output stream
    ///
    static void debug_print(const std::string& prefix, std::ostream& os) {

        using metkit::mars2grib::backend::concepts_::GeneralRegistry;

        os << prefix << " :: Select<" << typeid(Concept).name() << ">" << std::endl;
        os << prefix << " ::   is_any         : " << is_any << std::endl;
        os << prefix << " ::   count          : " << count << std::endl;
        os << prefix << " ::   variantIndices : [ ";

        for (std::size_t i = 0; i < ids.size(); ++i) {
            os << ids[i];
            if (i + 1 < ids.size())
                os << ", ";
        }
        os << " ]" << std::endl;


        os << prefix << " ::   variantNames : [ ";

        for (std::size_t i = 0; i < ids.size(); ++i) {
            std::size_t id    = ids[i];
            std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
            std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
            os << "\"" << cname << "::" << vname << "\"";
            if (i + 1 < ids.size())
                os << ", ";
        }

        os << " ]" << std::endl;
    }

    ///
    /// @brief Serialize the selector state to a JSON-like string.
    ///
    /// This method produces a compact JSON-style representation intended
    /// solely for debugging and diagnostics.
    ///
    /// @return String representation of the selector state
    ///
    /// @note
    /// The returned string is not guaranteed to be valid strict JSON and
    /// must not be used for machine parsing.
    ///
    static std::string debug_to_json() {

        using metkit::mars2grib::backend::concepts_::GeneralRegistry;

        std::ostringstream oss;

        oss << "{ \"Select\":{ \"typeId\": " << typeid(Concept).name();
        oss << ", \"is_any\": " << is_any;
        oss << ", \"count\":" << count;
        oss << ", \"variantIndices\": [";

        for (std::size_t i = 0; i < ids.size(); ++i) {
            oss << ids[i];
            if (i + 1 < ids.size())
                oss << ", ";
        }

        oss << "]";
        oss << ", \"variantIndices\": [";

        for (std::size_t i = 0; i < ids.size(); ++i) {
            std::size_t id    = ids[i];
            std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
            std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
            oss << "\"" << cname << "::" << vname << "\"";
            if (i + 1 < ids.size())
                oss << ", ";
        }

        oss << "]}}";

        return oss.str();
    }
};

}  // namespace metkit::mars2grib::backend::sections::resolver::dsl