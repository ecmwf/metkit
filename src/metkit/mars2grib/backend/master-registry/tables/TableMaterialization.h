#pragma once

/**
 * @file TableMaterialization.h
 * @brief Materialization helpers combining traversal and policies.
 *
 * This header defines helpers that combine:
 * - traversal skeletons
 * - applicability policies
 * - payload builders
 *
 * into fully materialized constexpr tables.
 *
 * @ingroup master_registry_tables
 */

#include <cstddef>
#include <type_traits>

#include "TablePolicies.h"

namespace metkit::mars2grib::backend::master_registry::tables {

/**
 * @brief Builder for conditional table entries.
 *
 * This helper encapsulates the pattern:
 * \code
 * if constexpr (Applicable<I,J,Tag>::value)
 *     return Payload::make<I,J>();
 * else
 *     return NullValue::value();
 * \endcode
 *
 * @tparam Applicable Applicability policy.
 * @tparam Payload Payload builder.
 * @tparam Null Null-value policy.
 * @tparam Tag Optional tag type (e.g. Variant).
 */
template <template <std::size_t, std::size_t, typename> class Applicable, typename Payload, typename Null, typename Tag>
struct ConditionalEntry {

    template <std::size_t I, std::size_t J>
    static constexpr auto make() {
        if constexpr (Applicable<I, J, Tag>::value) {
            return Payload::template make<I, J>();
        }
        else {
            return Null::value();
        }
    }
};

}  // namespace metkit::mars2grib::backend::master_registry::tables
