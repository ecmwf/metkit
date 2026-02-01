#pragma once

/**
 * @file TablePolicies.h
 * @brief Policy utilities controlling table materialization.
 *
 * This header defines small, composable policy types used by
 * table builders to decide:
 * - whether a table entry is applicable
 * - what value to emit when an entry is not applicable
 *
 * Policies are intentionally simple and constexpr-friendly.
 *
 * @ingroup master_registry_tables
 */

#include <cstddef>
#include <type_traits>

namespace metkit::mars2grib::backend::master_registry::tables {

/**
 * @brief Policy that marks every entry as applicable.
 *
 * Useful as a default or for tables without conditional logic.
 */
template <std::size_t I, std::size_t J, typename Tag>
struct AlwaysApplicable : std::true_type {};

/**
 * @brief Policy that marks every entry as non-applicable.
 *
 * Mostly useful for testing or as a sentinel.
 */
template <std::size_t I, std::size_t J, typename Tag>
struct NeverApplicable : std::false_type {};

/**
 * @brief Null-value policy.
 *
 * Provides a default value for non-applicable table entries.
 *
 * @tparam T Value type.
 */
template <typename T>
struct NullValue {
    static constexpr T value() noexcept { return T{}; }
};

/**
 * @brief Pointer-null policy specialization.
 */
template <typename T>
struct NullValue<T*> {
    static constexpr T* value() noexcept { return nullptr; }
};

}  // namespace metkit::mars2grib::backend::master_registry::tables
