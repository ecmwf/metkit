#pragma once

/**
 * @file MatcherRegistryBase.h
 * @brief Base for one-function-per-owner matcher tables.
 *
 * This header defines `MatcherRegistryBase`, which materializes a
 * compile-time array of function pointers (or callable pointers),
 * one per owner type.
 *
 * @par Contract for Owner types
 * Each owner type must define:
 * - `static constexpr MatcherFn matcher;`
 *
 * where `MatcherFn` is the function pointer type selected by the user.
 *
 * @ingroup master_registry_registries
 */

#include <array>
#include <cstddef>

#include "backend/master-registry/arrays/ArrayConcat.h"
#include "backend/master-registry/typelist/TypeList.h"

namespace metkit::mars2grib::backend::master_registry::registries {

namespace detail {

using namespace metkit::mars2grib::backend::master_registry::typelist;
using namespace metkit::mars2grib::backend::master_registry::arrays;

template <typename MatcherFn, typename Owner>
constexpr std::array<MatcherFn, 1> matcherScalar() {
    return {{static_cast<MatcherFn>(Owner::matcher)}};
}

template <typename MatcherFn, typename List>
struct BuildMatcherTable;

template <typename MatcherFn>
struct BuildMatcherTable<MatcherFn, TypeList<>> {
    static constexpr std::array<MatcherFn, 0> value() { return {}; }
};

template <typename MatcherFn, typename Head, typename... Tail>
struct BuildMatcherTable<MatcherFn, TypeList<Head, Tail...>> {
    static constexpr auto value() {
        return concat(matcherScalar<MatcherFn, Head>(), BuildMatcherTable<MatcherFn, TypeList<Tail...>>::value());
    }
};

}  // namespace detail

/**
 * @brief Base matcher table builder.
 *
 * @tparam OwnersT `typelist::TypeList<...>` of owner types.
 * @tparam MatcherFn Function pointer type stored in the table.
 */
template <typename OwnersT, typename MatcherFn>
struct MatcherRegistryBase {

    static constexpr std::size_t NOwners = OwnersT::size;

    /// ownerIndex -> matcher function pointer
    static inline constexpr auto matchers = detail::BuildMatcherTable<MatcherFn, OwnersT>::value();
};

}  // namespace metkit::mars2grib::backend::master_registry::registries
