/*
 * (C) Copyright 2026- ECMWF and individual contributors.
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
/// @brief Configuration options for the Mars2Mars conversion API.
///
/// This header defines the public configuration structure used to control the
/// behaviour of the Mars2Mars API error boundary.
///
/// `Options` is a fully materialised, strongly typed policy object. A caller may
/// construct it directly, or the API may populate it from another option source,
/// such as an `eckit::LocalConfiguration` or an initializer list.
///
/// Every member has an explicit default. Consequently, a default-constructed
/// `Options` object is always valid as an option dictionary.
///
/// The options control:
///
/// - whether a detailed structured error stack is saved to a file;
/// - where stack files are written when stack saving is enabled;
/// - whether the detailed structured error stack is printed to standard error.
///
/// @ingroup mars2mars_api
///
#pragma once

#include <string>

namespace metkit::mars2mars {

namespace defaults {

inline constexpr bool saveErrorStack                  = false;
inline constexpr const char* errorStackPath           = "./";
inline constexpr bool printErrorStackToStdErr         = false;
inline constexpr bool skipSection3                    = false;
inline constexpr bool tryFixBadInput_ZeroAccumulation = false;

}  // namespace defaults

///
/// @brief API error-handling options for Mars2Mars.
///
/// The structure is intentionally a plain aggregate:
///
/// - direct programmatic construction remains simple;
/// - every option can be copied into a stable policy snapshot;
/// - dictionary traits can expose the structure through the same typed access
///   API used for other Mars2Mars dictionaries;
/// - adding an option does not require virtual dispatch or ownership machinery.
///
struct Options {

    ///
    /// @brief Save the detailed API error stack to a file.
    ///
    /// When enabled, user-facing API failures persist the detailed structured
    /// exception stack to a file under `errorStackPath`, and the thrown public
    /// exception includes the full path to that file.
    ///
    /// @default false
    ///
    bool saveErrorStack = defaults::saveErrorStack;

    ///
    /// @brief Directory where API error-stack files are written.
    ///
    /// This path is used only when `saveErrorStack` is enabled.
    ///
    /// @default "./"
    ///
    std::string errorStackPath = defaults::errorStackPath;

    ///
    /// @brief Print the detailed API error stack to standard error.
    ///
    /// When enabled, user-facing API failures also emit the structured nested
    /// exception stack to `std::cerr`.
    ///
    /// @default false
    ///
    bool printErrorStackToStdErr = defaults::printErrorStackToStdErr;

    ///
    /// @brief Skip explicit encoding of GRIB Section 3.
    ///
    /// When enabled, the encoder does not encode the Grid Definition Section
    /// itself and leaves geometry handling to gridSpec/ecCodes.
    ///
    /// @default false
    ///
    bool skipSection3 = defaults::skipSection3;


    ///
    /// @brief Attempt to fix bad input when the accumulation period is zero.
    ///
    /// When enabled, the encoder attempts to fix bad input when the accumulation
    /// period is zero.
    ///
    /// @default false
    ///
    bool tryFixBadInput_ZeroAccumulation = defaults::tryFixBadInput_ZeroAccumulation;
};

}  // namespace metkit::mars2mars
