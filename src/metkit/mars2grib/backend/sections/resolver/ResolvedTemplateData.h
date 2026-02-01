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
 * @file ResolvedTemplateData.h
 * @brief Runtime container for parsed and resolved section recipes.
 *
 * This header defines `ResolvedTemplateData`, the **primary in-memory container**
 * used by the section-recipe resolution subsystem to store the outcome of
 * recipe parsing and template selection.
 *
 * A `ResolvedTemplateData` instance represents a fully resolved recipe entry
 * and encodes, in a compact and cache-friendly form:
 * - The GRIB **template number** to be applied
 * - The ordered list of **global concept-variant identifiers** that define
 *   the exact encoding logic for that template
 *
 * This structure is designed to be:
 * - Traversed frequently
 * - Compared and searched efficiently
 * - Passed through hot execution paths during encoding
 *
 * For this reason, the type is intentionally minimal, flat, and free of
 * ownership or dynamic allocation.
 *
 * Debug and introspection functionality is provided externally and granted
 * access via explicit friendship, ensuring that:
 * - The public interface remains minimal
 * - No debug-related code or symbols interfere with the hot path
 *
 * @ingroup mars2grib_backend_section_recipes
 */
#pragma once

// System includes
#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"

namespace metkit::mars2grib::backend::sections::resolver::dsl {


/**
 * @brief Main container for parsed and resolved section recipes.
 *
 * This structure is the **central storage unit** produced by the recipe
 * parsing and resolution pipeline.
 *
 * Each instance corresponds to a single resolved recipe and captures all
 * information required to:
 * - Identify the GRIB template to be used
 * - Drive the ordered execution of concept operations during encoding
 *
 * The container is explicitly optimized for **hot-path usage**:
 * - Fixed-capacity storage
 * - No dynamic memory allocation
 * - Trivial data layout
 *
 * Instances of this type are frequently accessed during recipe lookup
 * and encoding plan construction. As a consequence, no runtime validation
 * or defensive checks are performed inside the structure itself.
 *
 * Debug and introspection facilities are intentionally implemented as
 * external friend functions to avoid polluting the public API and to
 * ensure that debug-related code does not impact performance-critical
 * execution paths.
 *
 * The layout and semantics of this structure are considered part of a
 * **stable internal contract** for the section-recipe subsystem.
 */
struct ResolvedTemplateData {

    using GeneralRegistry = metkit::mars2grib::backend::concepts_::GeneralRegistry;

    /**
     * @brief Maximum number of concept variants that can be stored.
     *
     * This corresponds to the total number of registered concepts and
     * defines the fixed capacity of the container.
     */
    static constexpr std::size_t maxCapacity = GeneralRegistry::NConcepts;

    /**
     * @brief Ordered list of global concept-variant identifiers.
     *
     * Only the first @ref count entries are valid.
     */
    std::array<std::size_t, maxCapacity> variantIndices{};

    /**
     * @brief Number of active entries in @ref variantIndices.
     */
    std::size_t count{0};

    /**
     * @brief GRIB template number associated with this resolved recipe.
     */
    std::size_t templateNumber{0};
};


namespace debug {

inline void debug_print_ResolvedTemplateData(const ResolvedTemplateData& tdata, const std::string& prefix,
                                             std::ostream& os) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;

    os << prefix << " :: ResolvedTemplateData" << std::endl;
    os << prefix << " ::   templateNumber : " << tdata.templateNumber << std::endl;
    os << prefix << " ::   count          : " << tdata.count << std::endl;
    os << prefix << " ::   variantIndices : [ ";

    for (std::size_t i = 0; i < tdata.count; ++i) {
        os << tdata.variantIndices[i];
        if (i + 1 < tdata.count)
            os << ", ";
    }

    os << " ]" << std::endl;

    os << prefix << " ::   variantNames : [ ";

    for (std::size_t i = 0; i < tdata.count; ++i) {
        std::size_t id    = tdata.variantIndices[i];
        std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
        std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
        os << "\"" << cname << "::" << vname << "\"";
        if (i + 1 < tdata.count)
            os << ", ";
    }

    os << " ]" << std::endl;
}


inline std::string debug_convert_ResolvedTemplateData_to_json(const ResolvedTemplateData& tdata) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;

    std::ostringstream oss;

    oss << "{ \"ResolvedTemplateData\":{"
        << "\"templateNumber\":" << tdata.templateNumber << ", "
        << "\"count\": " << tdata.count << ", "
        << "\"variantIndices\":[";

    for (std::size_t i = 0; i < tdata.count; ++i) {
        oss << tdata.variantIndices[i];
        if (i + 1 < tdata.count)
            oss << ", ";
    }

    oss << "], ";

    oss << "\"variantNames\":[ ";

    for (std::size_t i = 0; i < tdata.count; ++i) {
        std::size_t id    = tdata.variantIndices[i];
        std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
        std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
        oss << "\"" << cname << "::" << vname << "\"";
        if (i + 1 < tdata.count)
            oss << ", ";
    }

    oss << " ]}}";

    return oss.str();
}

}  // namespace debug

}  // namespace metkit::mars2grib::backend::sections::resolver::dsl
