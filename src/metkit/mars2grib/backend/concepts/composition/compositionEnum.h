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
/// @file compositionEnum.h
/// @brief Definition of the `composition` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB `composition` concept
/// used by the mars2grib backend. It contains:
///
/// - the canonical concept name (`compositionName`)
/// - the exhaustive enumeration of supported composition variants (`CompositionType`)
/// - a compile-time typelist of all variants (`CompositionList`)
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
/// `composition.h` / `compositionOp` implementation.
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
/// @brief Canonical name of the `composition` concept.
///
/// This identifier is used:
/// - as the logical concept key in the concept registry
/// - for logging and debugging output
/// - to associate variants and capabilities with the `composition` concept
///
/// The value must remain stable across releases.
///
inline constexpr std::string_view compositionName{"composition"};


///
/// @brief Enumeration of all supported `composition` concept variants.
///
/// Each enumerator represents a distinct physical or semantic interpretation
/// of atmospheric composition handled by the encoder.
///
/// The numeric values of the enumerators are **not semantically relevant**;
/// they are required only to:
/// - provide a stable compile-time identifier
/// - allow array indexing and table generation
///
/// @note
/// This enumeration includes both state variables (e.g. chemical or aerosol
/// concentrations) and source-related representations.
///
/// @warning
/// Do not reorder existing enumerators, as they are used in compile-time
/// tables and registries.
///
enum class CompositionType : std::size_t {
    Chem = 0,
    Aerosol,
    AerosolOptical,
    ChemicalSource,
    AerosolOpticalSource,
    Default
};


///
/// @brief Compile-time list of all `composition` concept variants.
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
using CompositionList =
    ValueList<CompositionType::Chem, CompositionType::Aerosol, CompositionType::AerosolOptical,
              CompositionType::ChemicalSource, CompositionType::AerosolOpticalSource, CompositionType::Default>;


///
/// @brief Compile-time mapping from `CompositionType` to human-readable name.
///
/// This function returns the canonical string identifier associated
/// with a given composition variant.
///
/// The returned value is used for:
/// - logging and debugging output
/// - error reporting
/// - concept registry diagnostics
///
/// @tparam T Composition variant
/// @return String view identifying the variant
///
/// @note
/// The returned string must remain stable across releases, as it may
/// appear in logs, tests, and diagnostic output.
///
template <CompositionType T>
constexpr std::string_view compositionTypeName();

#define DEF(T, NAME)                                      \
    template <>                                           \
    constexpr std::string_view compositionTypeName<T>() { \
        return NAME;                                      \
    }

DEF(CompositionType::Chem, "chemical");
DEF(CompositionType::Aerosol, "aerosol");
DEF(CompositionType::AerosolOptical, "aerosolOptical");
DEF(CompositionType::ChemicalSource, "chemicalSource");
DEF(CompositionType::AerosolOpticalSource, "aerosolOpticalSource");
DEF(CompositionType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::concepts_
