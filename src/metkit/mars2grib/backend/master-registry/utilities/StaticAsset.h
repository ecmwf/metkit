#pragma once

/**
 * @file StaticAssert.h
 * @brief Readable compile-time assertion helpers.
 *
 * This header provides small utilities to improve the readability
 * and consistency of `static_assert` usage across the master-registry.
 *
 * These helpers are optional, but they make intent clearer and
 * error messages more uniform when used in heavily templated code.
 *
 * @ingroup master_registry_utilities
 */

#include <type_traits>

namespace metkit::mars2grib::backend::master_registry::utilities {

/**
 * @brief Compile-time assertion that a condition is true.
 *
 * This is a thin wrapper around `static_assert`, intended to
 * standardize error messages.
 *
 * @tparam Cond Boolean condition.
 * @tparam MsgType Type used only to introduce dependency (optional).
 */
template <bool Cond, typename MsgType = void>
struct StaticAssertTrue {
    static_assert(Cond, "StaticAssertTrue failed: condition is false");
};

/**
 * @brief Compile-time assertion that two types are the same.
 *
 * @tparam T First type.
 * @tparam U Second type.
 */
template <typename T, typename U>
struct StaticAssertSame {
    static_assert(std::is_same_v<T, U>, "StaticAssertSame failed: types are not identical");
};

/**
 * @brief Compile-time assertion that a type satisfies a trait.
 *
 * @tparam Trait Unary type trait (e.g. std::is_enum).
 * @tparam T Type to test.
 */
template <template <typename> class Trait, typename T>
struct StaticAssertTrait {
    static_assert(Trait<T>::value, "StaticAssertTrait failed: trait is not satisfied");
};

}  // namespace metkit::mars2grib::backend::master_registry::utilities
