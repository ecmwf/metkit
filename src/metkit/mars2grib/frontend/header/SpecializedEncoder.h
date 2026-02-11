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
/// @file SpecializedEncoder.h
/// @brief Fully specialized, hot-path GRIB header encoder.
///
/// This file defines the `SpecializedEncoder` class template, which represents
/// the **final, performance-critical encoding stage** of the mars2grib frontend.
///
/// The encoder is responsible for:
///
/// - owning a fully resolved GRIB header layout
/// - generating an optimized, immutable execution plan at construction time
/// - executing the plan in a dense, allocation-minimized hot path
///
/// Unlike generic or incremental encoders, this class **internalizes all
/// resolution work** and is designed to be invoked repeatedly with minimal
/// overhead once constructed.
///
/// -----------------------------------------------------------------------------
/// Architectural position
/// -----------------------------------------------------------------------------
///
/// The `SpecializedEncoder` sits at the boundary between:
///
/// - **frontend resolution** (concept selection, section layout, planning)
/// - **encoding execution** (dictionary mutation via callbacks)
///
/// All expensive or branching logic is assumed to have already happened before
/// this class is instantiated.
///
/// In particular:
///
/// - Section templates are already resolved
/// - Concept variants are already fixed
/// - The header layout is complete and immutable
///
/// As a result, `encode()` executes a **pre-compiled sequence of callbacks**
/// without any conditional logic beyond null-checks.
///
/// -----------------------------------------------------------------------------
/// Design goals
/// -----------------------------------------------------------------------------
///
/// This class is explicitly designed with the following goals:
///
/// 1. **Hot-path execution**
/// - No dynamic resolution
/// - No registry lookups
/// - No allocation except controlled cloning
///
/// 2. **Immutability**
/// - The layout and plan are `const`
/// - The encoder is thread-safe after construction
///
/// 3. **Move-only ownership**
/// - Layout data is moved into the encoder
/// - Copies are explicitly disabled
///
/// 4. **Failure transparency**
/// - Any failure is wrapped in a domain-specific exception
/// - Full diagnostic context is preserved via nested exceptions
///
/// -----------------------------------------------------------------------------
/// Typical lifecycle
/// -----------------------------------------------------------------------------
///
/// The intended usage pattern is:
///
/// @code
/// // One-time resolution phase
/// GribHeaderLayoutData layout = resolve_layout_or_throw(...);
///
/// // One-time specialization
/// SpecializedEncoder<MarsDict, ParDict, OptDict, OutDict> encoder(
/// std::move(layout)
/// );
///
/// // Hot-path execution (possibly millions of times)
/// auto grib = encoder.encode(mars, par, opt);
/// @endcode
///
/// -----------------------------------------------------------------------------
/// Template parameters
/// -----------------------------------------------------------------------------
///
/// @tparam MarsDict_t  Dictionary type containing MARS metadata
/// @tparam ParDict_t   Dictionary type containing parameter metadata
/// @tparam OptDict_t   Dictionary type containing encoding options
/// @tparam OutDict_t   Dictionary type representing the GRIB output
///
/// All dictionary types are expected to satisfy the `dict_traits` interface
/// used throughout mars2grib.
///

#pragma once

// System includes
#include <memory>
#include <utility>

// Project includes
#include "metkit/mars2grib/frontend/GribHeaderLayoutData.h"
#include "metkit/mars2grib/frontend/header/EncodingPlan.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::frontend::header {

///
/// @brief Fully specialized GRIB encoder.
///
/// This class represents a **fully materialized encoder instance**, where:
///
/// - the GRIB header layout is already resolved
/// - the execution plan is generated eagerly at construction
///
/// Once constructed, the encoder performs **no further planning or resolution**.
/// The `encode()` method executes a dense, pre-computed plan consisting of
/// concept setter callbacks organized by:
///
/// - encoding stage
/// - GRIB section
/// - concept callback
///
/// The encoder is **logically immutable** and safe to reuse across multiple
/// encoding calls with different input dictionaries.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
class SpecializedEncoder {
public:

    ///
    /// @brief Alias for the execution plan.
    ///
    /// The execution plan is a statically typed, nested container describing
    /// the exact sequence of operations required to populate the GRIB header.
    ///
    /// Its structure reflects the conceptual hierarchy:
    ///
    /// - stages
    /// - sections
    /// - concept callbacks
    ///
    /// The plan is generated once at construction time and never modified.
    ///
    using Plan_t =
        metkit::mars2grib::frontend::header::detail::EncodingPlan<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>;

    ///
    /// @brief Alias for resolved header layout data.
    ///
    /// This object describes the finalized layout of the GRIB header,
    /// including:
    ///
    /// - which sections are present
    /// - which templates are used
    /// - which concept variants are active
    ///
    /// It is assumed to be complete and internally consistent.
    ///
    using HeaderLayout_t = metkit::mars2grib::frontend::GribHeaderLayoutData;

    ///
    /// @brief Construct the encoder by internalizing the header layout.
    ///
    /// This constructor performs two tightly coupled operations:
    ///
    /// 1. Moves the resolved header layout into the encoder's internal state
    /// 2. Builds the optimized execution plan from the internalized layout
    ///
    /// The order of operations is intentional:
    ///
    /// - The layout is stored first
    /// - The plan factory reads directly from the stored layout
    ///
    /// @param[in] headerLayout
    /// Fully resolved header layout to be moved into the encoder.
    ///
    /// @throws Mars2GribException
    /// If plan construction fails due to inconsistent layout data.
    ///
    /// @note
    /// After construction, both the layout and the plan are immutable.
    ///
    explicit SpecializedEncoder(HeaderLayout_t&& headerLayout) :
        layout_{std::move(headerLayout)},
        plan_{detail::make_EncodingPlan_or_throw<MarsDict_t, ParDict_t, OptDict_t, OutDict_t>(layout_)} {}

    ///
    /// @name Special member functions
    /// @{
    ///

    ///
    /// @brief Copy construction is disabled.
    ///
    /// Copying the encoder would imply copying the layout and plan,
    /// which is both expensive and semantically undesirable.
    ///
    SpecializedEncoder(const SpecializedEncoder&) = delete;

    ///
    /// @brief Copy assignment is disabled.
    ///
    SpecializedEncoder& operator=(const SpecializedEncoder&) = delete;

    ///
    /// @brief Move construction is allowed.
    ///
    /// Enables efficient transfer of ownership when caching or
    /// storing encoders in containers.
    ///
    SpecializedEncoder(SpecializedEncoder&&) = default;

    ///
    /// @brief Move assignment is allowed.
    ///
    SpecializedEncoder& operator=(SpecializedEncoder&&) = default;

    ///
    /// @brief Destructor.
    ///
    /// Trivial destructor; all owned state is RAII-managed.
    ///
    ~SpecializedEncoder() = default;

    /// @}


    ///
    /// @brief Execute the encoding plan (hot path).
    ///
    /// This method performs the actual GRIB header encoding.
    ///
    /// Characteristics:
    ///
    /// - No layout resolution
    /// - No plan modification
    /// - No dynamic dispatch
    ///
    /// The algorithm is:
    ///
    /// 1. Create an initial GRIB sample dictionary
    /// 2. Iterate through the execution plan:
    /// - stages
    /// - sections
    /// - concept callbacks
    /// 3. Apply each non-null callback to the current dictionary
    /// 4. Clone the dictionary between stages as required
    ///
    /// @param[in] mars
    /// MARS metadata dictionary
    ///
    /// @param[in] par
    /// Parameter metadata dictionary
    ///
    /// @param[in] opt
    /// Encoding options dictionary
    ///
    /// @return
    /// A newly allocated dictionary containing the encoded GRIB header
    ///
    /// @throws Mars2GribEncoderException
    /// On any failure during encoding. The exception includes:
    /// - serialized input dictionaries
    /// - serialized header layout
    /// - full nested exception chain
    ///
    std::unique_ptr<OutDict_t> encode(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) const {

        using metkit::mars2grib::frontend::debug::debug_convert_GribHeaderLayoutData_to_json;
        using metkit::mars2grib::utils::dict_traits::clone_or_throw;
        using metkit::mars2grib::utils::dict_traits::dict_to_json;
        using metkit::mars2grib::utils::dict_traits::make_from_sample_or_throw;
        using metkit::mars2grib::utils::exceptions::Mars2GribEncoderException;

        try {
            auto samplePtr = make_from_sample_or_throw<OutDict_t>("GRIB2");

            // Encoding loop as a dnse set of optimized operations
            for (const auto& stage : plan_) {
                for (const auto& section : stage) {
                    for (const auto& conceptCallback : section) {
                        if (conceptCallback) {
                            conceptCallback(mars, par, opt, *samplePtr);
                        }
                    }
                }
                samplePtr = clone_or_throw<OutDict_t>(*samplePtr);
            }

            return samplePtr;
        }
        catch (...) {
            std::throw_with_nested(Mars2GribEncoderException(
                "Critical failure in SpecializedEncoder execution", dict_to_json<MarsDict_t>(mars),
                dict_to_json<ParDict_t>(par), dict_to_json<OptDict_t>(opt),
                debug_convert_GribHeaderLayoutData_to_json(layout_), Here()));
        }
    }

private:

    ///
    /// @brief Internalized header layout.
    ///
    /// Stored as `const` to enforce immutability after construction.
    ///
    /// @note
    /// This member must be declared before `plan_` so that it is
    /// fully initialized before being used by the plan factory.
    ///
    const HeaderLayout_t layout_;

    ///
    /// @brief Optimized execution plan.
    ///
    /// Generated eagerly from `layout_` during construction.
    /// Never modified thereafter.
    ///
    const Plan_t plan_;
};

}  // namespace metkit::mars2grib::frontend::header