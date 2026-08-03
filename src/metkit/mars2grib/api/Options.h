/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file Options.h
/// @brief Configuration options for the Mars2Grib encoding API.
///
/// This header defines the public configuration structure used to control the
/// behaviour of the Mars2Grib encoder.
///
/// `Options` is a fully materialised, strongly typed policy object. A caller may
/// construct it directly, or the API may populate it from another option source,
/// such as an `eckit::LocalConfiguration` or an initializer list.
///
/// Every member has an explicit default. Consequently, a default-constructed
/// `Options` object is always valid as an option dictionary, even though a later
/// deduction may reject a particular product when an opt-in policy required by
/// that product has not been enabled.
///
/// The options control:
///
/// - validation and override behaviour;
/// - metadata normalisation;
/// - GRIB encoding strategy;
/// - compatibility policies accepted by ProductTimeSpec;
/// - explicit ProductTimeSpec compatibility policies.
///
/// @ingroup mars2grib_api
///
#pragma once

namespace metkit::mars2grib {

namespace defaults {

inline constexpr bool applyChecks                   = true;
inline constexpr bool enableOverride                = false;
inline constexpr bool enableBitsPerValueCompression = false;
inline constexpr bool normalizeMars                 = false;
inline constexpr bool normalizeMisc                 = false;
inline constexpr bool fixMarsGrid                   = true;
inline constexpr bool skipSection3                  = false;

inline constexpr bool allowDefaultTimeIncrement                         = false;
inline constexpr bool allowZeroLengthFsWindow                           = false;
inline constexpr bool allowExtendedSetOfOperationsForZeroLengthFsWindow = false;
inline constexpr bool allowNonEnumeratedPositiveIntegerTimespanHours    = false;
inline constexpr bool allowRedundantTimeIncrement                       = true;
inline constexpr bool allowMissingTimespanForInstantProduct             = true;

}  // namespace defaults

///
/// @brief Encoding and ProductTimeSpec policy options for Mars2Grib.
///
/// The structure is intentionally a plain aggregate:
///
/// - direct programmatic construction remains simple;
/// - every option can be copied into a stable policy snapshot;
/// - dictionary traits can expose the structure through the same typed access
///   API used for other Mars2Grib dictionaries;
/// - adding an option does not require virtual dispatch or ownership machinery.
///
/// Unless stated otherwise, boolean options are disabled by default. This keeps
/// compatibility relaxations explicit and opt-in.
///
struct Options {

    // -------------------------------------------------------------------------
    // General encoder behaviour
    // -------------------------------------------------------------------------

    ///
    /// @brief Enable or disable input validation checks.
    ///
    /// When enabled, the encoder performs consistency and validity checks at
    /// selected critical points during encoding.
    ///
    /// Disabling this option may improve performance, but malformed or
    /// inconsistent input may then fail later and with less useful diagnostics.
    ///
    /// @default true
    ///
    bool applyChecks = defaults::applyChecks;

    ///
    /// @brief Enable metadata override semantics.
    ///
    /// When enabled, values provided through the parameter dictionary may
    /// override values resolved from the MARS dictionary.
    ///
    /// When disabled, conflicting overrides result in an error.
    ///
    /// @default false
    ///
    bool enableOverride = defaults::enableOverride;

    ///
    /// @brief Enable bits-per-value compression.
    ///
    /// When enabled, the encoder may select a bits-per-value packing strategy
    /// to reduce message size.
    ///
    /// This option affects the encoding strategy only. It does not alter the
    /// semantic meaning of the field.
    ///
    /// @default false
    ///
    bool enableBitsPerValueCompression = defaults::enableBitsPerValueCompression;

    ///
    /// @brief Enable semantic normalisation of the MARS dictionary.
    ///
    /// When enabled, the MARS request is sanitised against the active language
    /// definition before deductions are performed.
    ///
    /// @default false
    ///
    bool normalizeMars = defaults::normalizeMars;

    ///
    /// @brief Enable semantic normalisation of auxiliary metadata.
    ///
    /// When enabled, the auxiliary or Misc dictionary is sanitised against the
    /// active language definition before deductions are performed.
    ///
    /// @default false
    ///
    bool normalizeMisc = defaults::normalizeMisc;

    ///
    /// @brief Automatically normalise legacy MARS `grid` syntax.
    ///
    /// When enabled, the encoder converts supported legacy MARS grid
    /// specifications into the form expected by the GRIB geometry encoding.
    ///
    /// @default true
    ///
    bool fixMarsGrid = defaults::fixMarsGrid;

    ///
    /// @brief Skip explicit encoding of GRIB Section 3.
    ///
    /// When enabled, the encoder does not encode the Grid Definition Section
    /// itself and leaves geometry handling to gridSpec/ecCodes.
    ///
    /// @default false
    ///
    bool skipSection3 = defaults::skipSection3;

    // -------------------------------------------------------------------------
    // ProductTimeSpec policies
    // -------------------------------------------------------------------------

    ///
    /// @brief Allow a missing source time increment to be accepted.
    ///
    /// This option only enables the compatibility policy. ProductTimeSpec still
    /// decides whether the current product shape is eligible for this
    /// interpretation.
    ///
    /// In particular, enabling this option does not make every missing increment
    /// valid. Non-`ml` from-start single-loop products remain ineligible and
    /// still require an explicit source increment.
    ///
    /// @default false
    ///
    bool allowDefaultTimeIncrement = defaults::allowDefaultTimeIncrement;

    ///
    /// @brief Allow a zero-length from-start statistical window.
    ///
    /// When disabled, a from-start product whose resolved step is zero is
    /// rejected. When enabled, ProductTimeSpec may construct the explicitly
    /// supported zero-length from-start statistical representation.
    ///
    /// @default false
    ///
    bool allowZeroLengthFsWindow = defaults::allowZeroLengthFsWindow;

    /// @brief Allow extended set of operations for zero-length from-start window.
    ///
    /// When disabled, only accumulation is allowed at step zero. When enabled,
    /// ProductTimeSpec may construct the explicitly supported zero-length from-start
    /// statistical representation for average, minimum, and maximum operations.
    ///
    /// @default false
    ///
    bool allowExtendedSetOfOperationsForZeroLengthFsWindow =
        defaults::allowExtendedSetOfOperationsForZeroLengthFsWindow;

    ///
    /// @brief Allow positive integer-hour `timespan` values not enumerated by
    /// the active MARS language.
    ///
    /// Integer-valued `timespan` is interpreted as hours. By default, the value
    /// must belong to the language-defined supported set. Enabling this option
    /// allows any strictly positive integer number of hours, subject to the
    /// remaining ProductTimeSpec checks.
    ///
    /// @default false
    ///
    bool allowNonEnumeratedPositiveIntegerTimespanHours = defaults::allowNonEnumeratedPositiveIntegerTimespanHours;

    ///
    /// @brief Allow explicit time increments that are semantically redundant.
    ///
    /// This compatibility policy is used for cases where the product semantics
    /// do not require an increment, but an input dictionary nevertheless
    /// provides one. ProductTimeSpec remains responsible for deciding which
    /// product shapes may ignore such a redundant value.
    ///
    /// @default false
    ///
    bool allowRedundantTimeIncrement = defaults::allowRedundantTimeIncrement;

    ///
    /// @brief Allow a missing `timespan` to represent an instant product.
    ///
    /// The normative instant representation uses `timespan="none"`. Enabling
    /// this option accepts the compatibility representation in which both
    /// `timespan` and `stattype` are absent.
    ///
    /// @default false
    ///
    bool allowMissingTimespanForInstantProduct = defaults::allowMissingTimespanForInstantProduct;
};

}  // namespace metkit::mars2grib
