#pragma once

/**
 * @file DependentFalse.h
 * @brief Helper for dependent compile-time failures.
 *
 * This header defines `dependent_false`, a standard metaprogramming
 * utility used to trigger `static_assert(false)` in a *dependent*
 * context.
 *
 * In templates, writing `static_assert(false)` directly is ill-formed,
 * because the assertion is evaluated immediately. `dependent_false`
 * delays evaluation until template instantiation.
 *
 * Typical usage:
 * \code
 * template <typename T>
 * struct Foo {
 *     static_assert(dependent_false<T>::value,
 *                   "Unsupported type T");
 * };
 * \endcode
 *
 * @ingroup master_registry_utilities
 */

#include <type_traits>

namespace metkit::mars2grib::backend::master_registry::utilities {

/**
 * @brief Always-false type dependent on template parameters.
 *
 * @tparam Ts Dummy parameter pack to introduce dependency.
 */
template <typename... Ts>
struct dependent_false : std::false_type {};

}  // namespace metkit::mars2grib::backend::master_registry::utilities
