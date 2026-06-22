/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file mars2marsExceptions.h
/// @brief Unified exception hierarchy for the mars2mars framework.
///
/// This header defines the complete exception model used across
/// mars2mars, covering:
///
/// - Generic infrastructure errors
/// - Layer-specific failures (matcher, rules, validation, tables, deduction)
/// - Concept execution failures (with contextual metadata)
/// - Encoder failures (with serialized dictionary state)
///
/// The hierarchy is designed with the following goals:
///
/// - Strong contextual diagnostics
/// - Support for nested exception propagation
/// - Structured debug frame printing
/// - Clear separation between backend and frontend layers
///
/// All exceptions ultimately derive from `eckit::Exception`,
/// ensuring compatibility with the broader ECMWF ecosystem.
///
/// Nested exception support allows propagation chains to be
/// printed in a structured stack-like format.
///
/// @ingroup mars2mars_utils
#pragma once

// System includes
#include <exception>
#include <optional>
#include <string>
#include <typeinfo>

// Project includes
#include "eckit/exception/Exceptions.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2mars/utils/generalUtils.h"


namespace metkit::mars2mars::utils::exceptions {

/// @brief Base exception for mars2mars.
///
/// This is the root exception type for most mars2mars failures.
/// It:
///
/// - Inherits from `eckit::Exception`
/// - Supports nested exceptions via `std::nested_exception`
/// - Provides structured frame printing
///
/// Derived exceptions typically extend this class with
/// additional contextual metadata.
///
/// The `printFrame()` method is designed to be used by
/// extended stack printers.
class Mars2marsGenericException : public eckit::Exception, public std::nested_exception {
public:

    Mars2marsGenericException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        eckit::Exception(reason, loc) {}

    virtual ~Mars2marsGenericException() = default;

    virtual void printFrame(const std::string& pad, std::ostream& os) const {

        const auto& loc = location();

        os << pad << "+ file:     " << loc.file() << "\n"
           << pad << "+ function: " << loc.func() << "\n"
           << pad << "+ line:     " << loc.line() << "\n"
           << pad << "+ link:     " << loc.file() << ":" << loc.line() << "\n"
           << pad << "+ message:  " << what() << "\n";
    }
};


/// @brief Exception raised in the rules layer.
///
/// Used when evaluating rule-based logic fails.
///
/// This class derives directly from `eckit::Exception` and
/// supports nested exceptions.
class Mars2marsRulesException : public eckit::Exception, public std::nested_exception {
public:

    Mars2marsRulesException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        eckit::Exception(reason, loc) {}
};

/// @brief Exception raised in the dictionary access layer.
///
/// Used when dictionary validation or access fails.
/// Inherits structured printing from the generic exception.
class Mars2marsDictException : public Mars2marsGenericException {
public:

    Mars2marsDictException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2marsGenericException(reason, loc) {}
};


inline constexpr int tabSize  = 4;
inline constexpr int lineSize = 120;

inline std::string indent(std::size_t level) {
    return std::string(level * tabSize, ' ');
}

/// @brief Print structured exception stack with detailed frames.
///
/// For each nested exception frame:
///
/// - Prints file, function, line, and message (if available)
/// - Prints additional metadata for specialized exceptions
///
/// This function detects `Mars2marsGenericException`
/// via `dynamic_cast` and calls `printFrame()`
/// to extract structured information.
///
/// Nested exceptions are recursively printed.
///
/// @param e      Root exception
/// @param level  Indentation level
/// @param frame  Frame counter
inline void printExtendedStack(const std::exception& e, std::ostream& os, std::size_t level = 0,
                               std::size_t frame = 1) {

    const std::string pad = indent(level);

    os << pad << "+ " << std::string(lineSize, '=') << std::endl
       << pad << "+ frame " << frame << std::endl
       << pad << "+ " << std::string(lineSize, '-') << std::endl;

    if (const auto* me = dynamic_cast<const Mars2marsGenericException*>(&e)) {
        me->printFrame(pad, os);
    }
    else {
        os << pad << "+ message: " << e.what() << std::endl;
    }

    os << pad << "+ " << std::string(lineSize, '+') << std::endl;

    const auto* nested = dynamic_cast<const std::nested_exception*>(&e);

    if (nested == nullptr) {
        return;
    }

    const std::exception_ptr nestedPtr = nested->nested_ptr();

    if (!nestedPtr) {
        return;
    }

    try {
        std::rethrow_exception(nestedPtr);
    }
    catch (const std::exception& nestedException) {
        printExtendedStack(nestedException, os, level + 1, frame + 1);
    }
    catch (...) {
        os << std::string(level + 1, ' ') << "[nested non-std exception]" << std::endl;
    }
}

}  // namespace metkit::mars2mars::utils::exceptions
