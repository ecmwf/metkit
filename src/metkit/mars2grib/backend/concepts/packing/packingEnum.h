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
/// @file packingEnum.h
/// @brief Definition of the `packing` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB `packing` concept
/// used by the mars2grib backend. It contains:
///
/// - the canonical concept name (`packingName`)
/// - the enumeration of supported packing variants (`PackingType`)
/// - a compile-time typelist of all variants (`PackingList`)
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
/// `packing.h` / `packingOp` implementation.
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
/// @brief Canonical name of the `packing` concept.
///
/// This identifier is used:
/// - as the logical concept key in the concept registry
/// - for logging and debugging output
/// - to associate variants and capabilities with the `packing` concept
///
/// The value must remain stable across releases.
///
inline constexpr std::string_view packingName{"packing"};


///
/// @brief Enumeration of all supported `packing` concept variants.
///
/// Each enumerator represents a specific GRIB packing or compression
/// strategy used to encode field values.
///
/// The numeric values of the enumerators are **not semantically relevant**;
/// they are required only to:
/// - provide a stable compile-time identifier
/// - allow array indexing and table generation
///
/// @note
/// This enumeration includes both grid-point and spectral packing
/// representations, as well as a default fallback.
///
/// @warning
/// Do not reorder existing enumerators, as they are used in compile-time
/// tables and registries.
///
enum class PackingType : std::size_t {
    Simple = 0,
    Ccsds,
    SpectralComplex,
    Default
};


///
/// @brief Compile-time list of all `packing` concept variants.
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
using PackingList =
    ValueList<PackingType::Simple, PackingType::Ccsds, PackingType::SpectralComplex, PackingType::Default>;


///
/// @brief Compile-time mapping from `PackingType` to human-readable name.
///
/// This function returns the canonical string identifier associated
/// with a given packing variant.
///
/// The returned value is used for:
/// - logging and debugging output
/// - error reporting
/// - concept registry diagnostics
///
/// @tparam T Packing variant
/// @return String view identifying the variant
///
/// @note
/// The returned string must remain stable across releases, as it may
/// appear in logs, tests, and diagnostic output.
///
template <PackingType T>
constexpr std::string_view packingTypeName();

#define DEF(T, NAME)                                  \
    template <>                                       \
    constexpr std::string_view packingTypeName<T>() { \
        return NAME;                                  \
    }

DEF(PackingType::Simple, "simple");
DEF(PackingType::Ccsds, "ccsds");
DEF(PackingType::SpectralComplex, "spectral_complex");
DEF(PackingType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::concepts_
