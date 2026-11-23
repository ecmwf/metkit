// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file clusterEnum.h
/// @brief Definition of the `cluster` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB `cluster` concept
/// used by the mars2grib backend. It contains:
///
/// - the canonical concept name (`clusterName`)
/// - the exhaustive enumeration of supported cluster variants (`ClusterType`)
/// - a compile-time typelist of all variants (`ClusterList`)
/// - a compile-time mapping from variant to string identifier
///
/// This file intentionally contains **no runtime logic** and **no encoding
/// behavior**. Its sole purpose is to provide compile-time metadata used by:
///
/// - the concept registry
/// - compile-time table generation
/// - logging and diagnostics
/// - static validation of concept variants
///
/// @note
/// This header is part of the **concept definition layer**.
/// Runtime behavior is implemented separately in the corresponding
/// `clusterEncoding.h` implementation.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <cstdint>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <auto... Vals>
using ValueList = metkit::mars2grib::backend::compile_time_registry_engine::ValueList<Vals...>;

///
/// @brief Canonical name of the `cluster` concept.
///
/// This identifier is used:
/// - as the logical concept key in the concept registry
/// - for logging and debugging output
/// - to associate variants and capabilities with the `cluster` concept
///
/// The value must remain stable across releases.
///
inline constexpr std::string_view clusterName{"cluster"};


///
/// @brief Enumeration of all supported `cluster` concept variants.
///
/// Each enumerator represents a specific cluster product description.
///
/// The numeric values of the enumerators are **not semantically relevant**;
/// they are required only to:
/// - provide a stable compile-time identifier
/// - allow array indexing and table generation
///
/// @warning
/// Do not reorder existing enumerators, as they are used in compile-time
/// tables and registries.
///
enum class ClusterType : std::size_t {
    Rectangular = 0  ///< Cluster products over a rectangular area
};


///
/// @brief Compile-time list of all `cluster` concept variants.
///
/// This typelist is used to:
/// - generate concept capability tables at compile time
/// - register all supported variants in the concept registry
/// - enable static iteration over variants without runtime overhead
///
/// @note
/// The order of this list must match the intended iteration order
/// for registry construction and diagnostics.
///
using ClusterList = ValueList<ClusterType::Rectangular>;


///
/// @brief Compile-time mapping from `ClusterType` to human-readable name.
///
/// This function returns the canonical string identifier associated
/// with a given cluster variant.
///
/// The returned value is used for:
/// - logging and debugging output
/// - error reporting
/// - concept registry diagnostics
///
/// @tparam T Cluster variant
/// @return String view identifying the variant
///
/// @note
/// The returned string must remain stable across releases, as it may
/// appear in logs, tests, and diagnostic output.
///
template <ClusterType T>
constexpr std::string_view clusterTypeName();

#define DEF(T, NAME)                                  \
    template <>                                       \
    constexpr std::string_view clusterTypeName<T>() { \
        return NAME;                                  \
    }

DEF(ClusterType::Rectangular, "rectangular");

#undef DEF

}  // namespace metkit::mars2grib::backend::concepts_
