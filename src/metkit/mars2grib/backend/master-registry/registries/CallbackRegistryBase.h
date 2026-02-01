#pragma once

/**
 * @file CallbackRegistryBase.h
 * @brief Base for [Value][Stage][Section] callback dispatch tables.
 *
 * This header defines `CallbackRegistryBase`, a reusable base that builds:
 *
 * \code
 * dispatch[value][stage][section] -> function pointer (or null)
 * \endcode
 *
 * where `value` is a compact 0-based enum (or integer domain) of size `NValues`.
 *
 * The table is built fully at compile time using index-sequence expansion.
 * Applicability is expressed as a policy:
 *
 * - if Applicable<Stage, Section, Value>::value -> store pointer to Op<...>
 * - else -> store nullptr
 *
 * @par Contract
 * - ValueEnum is 0-based contiguous, size = NValues
 * - Op is a function template:
 *   `Op<Stage, Section, Value, Mars, Par, Opt, Out>(...)`
 * - Applicable is a bool trait:
 *   `Applicable<Stage, Section, Value>::value`
 *
 * @ingroup master_registry_registries
 */

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace metkit::mars2grib::backend::master_registry::registries {

/**
 * @brief Function pointer type used in dispatch tables.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using CallbackFn = void (*)(const MarsDict_t&, const ParDict_t&, const OptDict_t&, OutDict_t&);

namespace detail {

/**
 * @brief Entry selector with applicability policy.
 */
template <template <std::size_t, std::size_t, auto> class Applicable,
          template <std::size_t, std::size_t, auto, class, class, class, class> class Op, std::size_t Stage,
          std::size_t Sec, auto Value, class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
static constexpr CallbackFn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t> makeEntry() {
    if constexpr (Applicable<Stage, Sec, Value>::value) {
        return &Op<Stage, Sec, Value, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
    }
    else {
        return nullptr;
    }
}

template <template <std::size_t, std::size_t, auto> class Applicable,
          template <std::size_t, std::size_t, auto, class, class, class, class> class Op, auto Value, class MarsDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t, std::size_t Stage, std::size_t... Secs, std::size_t NSec>
static constexpr std::array<CallbackFn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, NSec> makeStageRowImpl(
    std::index_sequence<Secs...>) {
    return {{makeEntry<Applicable, Op, Stage, Secs, Value, MarsDict_t, ParDict_t, OptDict_t, OutDict_t>()...}};
}

template <template <std::size_t, std::size_t, auto> class Applicable,
          template <std::size_t, std::size_t, auto, class, class, class, class> class Op, auto Value, class MarsDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t, std::size_t... Stages, std::size_t NStage,
          std::size_t NSec>
static constexpr std::array<std::array<CallbackFn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>, NSec>, NStage>
makeStageSecTableImpl(std::index_sequence<Stages...>) {
    return {{makeStageRowImpl<Applicable, Op, Value, MarsDict_t, ParDict_t, OptDict_t, OutDict_t, Stages, NSec>(
        std::make_index_sequence<NSec>{})...}};
}

}  // namespace detail

/**
 * @brief Base callback dispatch table builder.
 *
 * @tparam ValueEnum Enum type used to index values (must be compact 0..NValues-1).
 * @tparam NValues Number of values in the value domain.
 * @tparam NStage Number of stages.
 * @tparam NSec Number of sections.
 * @tparam Applicable Applicability policy: `Applicable<Stage,Sec,Value>::value`.
 * @tparam Op Callback template: `Op<Stage,Sec,Value,Mars,Par,Opt,Out>`.
 * @tparam MarsDict_t Dictionary types.
 * @tparam ParDict_t Dictionary types.
 * @tparam OptDict_t Dictionary types.
 * @tparam OutDict_t Dictionary types.
 */
template <typename ValueEnum, std::size_t NValues, std::size_t NStage, std::size_t NSec,
          template <std::size_t, std::size_t, auto> class Applicable,
          template <std::size_t, std::size_t, auto, class, class, class, class> class Op, class MarsDict_t,
          class ParDict_t, class OptDict_t, class OutDict_t>
struct CallbackRegistryBase {

    using FnT = CallbackFn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;

    using StageSecTable = std::array<std::array<FnT, NSec>, NStage>;

    using DispatchTable = std::array<StageSecTable, NValues>;

    /**
     * @brief Materialize Stage×Section table for a given Value.
     */
    template <ValueEnum V>
    static constexpr StageSecTable makeStageSecTable() {
        return detail::makeStageSecTableImpl<Applicable, Op, static_cast<decltype(static_cast<std::size_t>(V))>(V),
                                             MarsDict_t, ParDict_t, OptDict_t, OutDict_t,
                                             /*Stages...*/ 0, NStage, NSec>(std::make_index_sequence<NStage>{});
    }

private:

    template <std::size_t... Vs>
    static constexpr DispatchTable makeDispatchImpl(std::index_sequence<Vs...>) {
        return {{detail::makeStageSecTableImpl<
            Applicable, Op, static_cast<decltype(static_cast<std::size_t>(ValueEnum{}))>(static_cast<ValueEnum>(Vs)),
            MarsDict_t, ParDict_t, OptDict_t, OutDict_t,
            /*Stages...*/ 0, NStage, NSec>(std::make_index_sequence<NStage>{})...}};
    }

public:

    /**
     * @brief Fully materialized dispatch table.
     *
     * Indexing:
     * - first dimension: value id (0..NValues-1)
     * - second: stage
     * - third: section
     */
    static inline constexpr DispatchTable dispatch = makeDispatchImpl(std::make_index_sequence<NValues>{});
};

}  // namespace metkit::mars2grib::backend::master_registry::registries
