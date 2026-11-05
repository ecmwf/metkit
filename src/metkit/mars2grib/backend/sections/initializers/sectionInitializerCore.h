// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file sectionInitializerTypes.h
/// @brief Core type definitions for GRIB section initializer registries.
///
/// This header defines the **fundamental type aliases** used by the
/// mars2grib backend to describe section initializer registries.
///
/// Section initializers are lightweight, stateless functions responsible for
/// populating GRIB sections based on the resolved dictionaries produced by
/// the frontend and concept layers.
///
/// This file provides:
/// - a canonical function pointer type (`Fn`) for section initializers
/// - a registry entry type (`Entry`) pairing a template number with an initializer
///
/// These types are used to build compile-time or static registries mapping
/// GRIB template numbers to the corresponding initialization routines.
///
/// This file contains **no logic**, **no state**, and **no runtime behavior**.
/// It is purely a type-definition header.
///
/// @ingroup mars2grib_backend_sections
///
#pragma once

// System includes
#include <cstddef>
#include <utility>

#include "metkit/mars2grib/backend/concepts/EncodingCallbacksRegistry.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::sections::initializers {

///
/// @brief Function pointer type for GRIB section initializers.
///
/// This alias defines the canonical signature for all section initializer
/// functions used by the mars2grib backend.
///
/// A section initializer:
/// - consumes read-only frontend dictionaries (`Mars`, `Param`, `Options`)
/// - mutates the output GRIB dictionary corresponding to a specific section
///
/// Initializers are invoked by the encoder once the appropriate GRIB
/// template has been selected.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the output GRIB dictionary
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using Fn = metkit::mars2grib::backend::compile_time_registry_engine::Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;

///
/// @brief Registry entry associating a GRIB template number with an initializer.
///
/// An `Entry` represents a single row in a section initializer registry.
/// It binds:
/// - a GRIB template number (e.g. Product Definition Template, Grid Definition Template)
/// - the corresponding section initializer function
///
/// Registries composed of these entries are used to:
/// - select the correct initializer based on the resolved template number
/// - dispatch section initialization at runtime with zero dynamic allocation
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the output GRIB dictionary
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
struct Entry {
    using Fn_t = Fn<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;
    std::size_t templateNumber;
    Fn_t callback;
};
}  // namespace metkit::mars2grib::backend::sections::initializers