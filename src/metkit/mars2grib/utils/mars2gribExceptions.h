/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file mars2gribExceptions.h
/// @brief Unified exception hierarchy for the mars2grib framework.
///
/// This header defines the complete exception model used across
/// mars2grib, covering:
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
/// @ingroup mars2grib_utils
#pragma once

// System includes
#include <exception>
#include <optional>
#include <string>
#include <typeinfo>

// Project includes
#include "metkit/mars2grib/utils/generalUtils.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/config/LibMetkit.h"


namespace metkit::mars2grib::utils::exceptions {

/// @brief Base exception for mars2grib.
///
/// This is the root exception type for most mars2grib failures.
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
class Mars2GribGenericException : public eckit::Exception, public std::nested_exception {
public:

    Mars2GribGenericException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        eckit::Exception(reason, loc) {}

    virtual ~Mars2GribGenericException() = default;

    virtual void printFrame(const std::string& pad) const {

        const auto& loc = location();

        LOG_DEBUG_LIB(LibMetkit) << pad << "+ file:     " << loc.file() << "\n"
                                 << pad << "+ function: " << loc.func() << "\n"
                                 << pad << "+ line:     " << loc.line() << "\n"
                                 << pad << "+ link:     " << loc.file() << ":" << loc.line() << "\n"
                                 << pad << "+ message:  " << what() << "\n";
    }
};


/// @brief Exception raised during matcher evaluation.
///
/// This exception is used when resolving whether a concept
/// should be activated based on MARS input.
///
/// It may optionally carry:
///
/// - `param`   : parameter identifier
/// - `levtype` : level type
///
/// All metadata fields are optional and printed only if defined.
///
/// This exception extends the generic exception with matcher-specific
/// diagnostic context.
class Mars2GribMatcherException : public Mars2GribGenericException {
public:

    Mars2GribMatcherException(long param, const std::string& levtype, std::string reason,
                              const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc), param_{std::to_string(param)}, levtype_{levtype} {}

    Mars2GribMatcherException(const std::string& levtype, std::string reason,
                              const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc), param_{std::nullopt}, levtype_{levtype} {}

    Mars2GribMatcherException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc), param_{std::nullopt}, levtype_{std::nullopt} {}

    Mars2GribMatcherException(long param, std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc), param_{std::to_string(param)}, levtype_{std::nullopt} {}

    const std::string levtype() const { return levtype_.has_value() ? levtype_.value() : "undefined"; }
    const std::string param() const { return param_.has_value() ? param_.value() : "undefined"; }

    void printFrame(const std::string& pad) const override {

        Mars2GribGenericException::printFrame(pad);
        if (param_) {
            LOG_DEBUG_LIB(LibMetkit) << pad << "+ param:   " << param() << std::endl;
        }
        if (levtype_) {
            LOG_DEBUG_LIB(LibMetkit) << pad << "+ levtype:    " << levtype() << std::endl;
        }
    };


private:

    std::optional<std::string> param_;
    std::optional<std::string> levtype_;
};

/// @brief Exception raised in the rules layer.
///
/// Used when evaluating rule-based logic fails.
///
/// This class derives directly from `eckit::Exception` and
/// supports nested exceptions.
class Mars2GribRulesException : public eckit::Exception, public std::nested_exception {
public:

    Mars2GribRulesException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        eckit::Exception(reason, loc) {}
};

/// @brief Exception raised in the dictionary access layer.
///
/// Used when dictionary validation or access fails.
/// Inherits structured printing from the generic exception.
class Mars2GribDictException : public Mars2GribGenericException {
public:

    Mars2GribDictException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc) {}
};

/// @brief Exception raised in the validation layer.
///
/// Intended for semantic validation errors.
/// Currently does not add additional metadata beyond the base class.
class Mars2GribValidationException : public Mars2GribGenericException {
public:

    Mars2GribValidationException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc) {}
};

/// @brief Exception raised in table resolution logic.
///
/// Used when GRIB table lookup or interpretation fails.
class Mars2GribTableException : public Mars2GribGenericException {
public:

    Mars2GribTableException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc) {}
};

/// @brief Exception raised in the deduction layer.
///
/// Used when derived values cannot be computed or inferred
/// from the provided dictionaries.
class Mars2GribDeductionException : public Mars2GribGenericException {
public:

    Mars2GribDeductionException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc) {}
};

/// @brief Exception raised during concept execution.
///
/// This is the most context-rich exception in the hierarchy.
/// It carries:
///
/// - Concept name
/// - Concept variant
/// - Encoding stage
/// - GRIB section
///
/// This allows precise identification of:
///
/// - Which concept failed
/// - Under which stage
/// - In which section
///
/// The metadata is optional and printed only if present.
///
/// This exception is typically constructed via helper macros:
///
/// - `MARS2GRIB_CONCEPT_THROW`
/// - `MARS2GRIB_CONCEPT_RETHROW`
///
/// Those macros automatically inject compile-time metadata.
class Mars2GribConceptException : public Mars2GribGenericException {
public:

    Mars2GribConceptException(std::string name, std::string variant, std::string stage, std::string section,
                              std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc),
        conceptName_(std::move(name)),
        conceptVariant_(std::move(variant)),
        stage_(std::move(stage)),
        section_(std::move(section)) {}

    const std::optional<std::string>& conceptName() const { return conceptName_; }
    const std::optional<std::string>& conceptVariant() const { return conceptVariant_; }
    const std::optional<std::string>& stage() const { return stage_; }
    const std::optional<std::string>& section() const { return section_; }

    void printFrame(const std::string& pad) const override {

        Mars2GribGenericException::printFrame(pad);

        auto print_opt = [&](const char* k, const std::optional<std::string>& v) {
            if (v)
                LOG_DEBUG_LIB(LibMetkit) << pad << "+ " << k << ": " << *v << "\n";
        };

        print_opt("concept", conceptName_);
        print_opt("variant", conceptVariant_);
        print_opt("stage", stage_);
        print_opt("section", section_);
    }

private:

    std::optional<std::string> conceptName_;
    std::optional<std::string> conceptVariant_;
    std::optional<std::string> stage_;
    std::optional<std::string> section_;
};

/// @brief Exception raised in the encoder layer.
///
/// This exception captures serialized diagnostic state,
/// including JSON representations of:
///
/// - MARS dictionary
/// - Parameter dictionary
/// - Options dictionary
/// - Encoder configuration
///
/// It is intended for high-level failure reporting where
/// complete encoding context must be preserved.
///
/// The diagnostic information is printed in structured form
/// via `printFrame()`.
class Mars2GribEncoderException : public Mars2GribGenericException {
public:

    Mars2GribEncoderException(std::string reason, std::string marsDict_json, std::string parDict_json,
                              std::string optDict_json, std::string encoderCfg_json,
                              const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc),
        marsDict_json_(std::move(marsDict_json)),
        parDict_json_(std::move(parDict_json)),
        optDict_json_(std::move(optDict_json)),
        encoderCfg_json_(std::move(encoderCfg_json)) {}

    const std::string& marsDict_json() const { return marsDict_json_; }
    const std::string& geoDict_json() const { return geoDict_json_; }
    const std::string& parDict_json() const { return parDict_json_; }
    const std::string& optDict_json() const { return optDict_json_; }
    const std::string& encoderCfg_json() const { return encoderCfg_json_; }

    void printFrame(const std::string& pad) const override {

        Mars2GribGenericException::printFrame(pad);

        LOG_DEBUG_LIB(LibMetkit) << pad << "+ marsDict:   " << marsDict_json_ << "\n"
                                 << pad << "+ parDict:    " << parDict_json_ << "\n"
                                 << pad << "+ optDict:    " << optDict_json_ << "\n"
                                 << pad << "+ encoderCfg: " << encoderCfg_json_ << "\n";
    }

private:

    const std::string marsDict_json_;
    const std::string geoDict_json_;
    const std::string parDict_json_;
    const std::string optDict_json_;
    const std::string encoderCfg_json_;
};

/// @brief Recursively print nested exception stack.
///
/// Prints:
///
/// - Exception type
/// - Exception message
/// - Nested exceptions (if any)
///
/// The structure is indented according to nesting level.
///
/// This function does not use the structured frame printer.
/// For detailed frames, use `printExtendedStack`.
///
/// @param e      Root exception
/// @param os     Output stream
/// @param level  Initial indentation level
inline void printExceptionStack(const std::exception& e, std::ostream& os, std::size_t level = 0) {
    const std::string indent(level * 2, ' ');

    // stampa tipo + messaggio
    os << indent << "- [" << typeid(e).name() << "] " << e.what() << '\n';

    // verifica eccezione annidata
    try {
        std::rethrow_if_nested(e);
    }
    catch (const std::exception& nested) {
        printExceptionStack(nested, os, level + 1);
    }
    catch (...) {
        os << indent << "  - [unknown non-std exception]\n";
    }
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
/// This function detects `Mars2GribGenericException`
/// via `dynamic_cast` and calls `printFrame()`
/// to extract structured information.
///
/// Nested exceptions are recursively printed.
///
/// @param e      Root exception
/// @param level  Indentation level
/// @param frame  Frame counter
inline void printExtendedStack(const std::exception& e, std::size_t level = 0, std::size_t frame = 1) {

    const std::string pad = indent(level);

    LOG_DEBUG_LIB(LibMetkit) << pad << "+ " << std::string(lineSize, '=') << std::endl
                             << pad << "+ frame " << frame << std::endl
                             << pad << "+ " << std::string(lineSize, '-') << std::endl;

    if (const auto* me = dynamic_cast<const Mars2GribGenericException*>(&e)) {
        me->printFrame(pad);
    }
    else {
        LOG_DEBUG_LIB(LibMetkit) << pad << "+ message: " << e.what() << std::endl;
    }

    LOG_DEBUG_LIB(LibMetkit) << pad << "+ " << std::string(lineSize, '+') << std::endl;

    try {
        std::rethrow_if_nested(e);
    }
    catch (const std::exception& nested) {
        printExtendedStack(nested, level + 1, frame + 1);
    }
}

/// @brief Join a vector of long values into a formatted string.
///
/// Output format:
/// `{v1, v2, v3}`
///
/// Intended for diagnostic message construction.
inline std::string joinNumbers(const std::vector<long>& vec) {
    std::string s{"{"};
    for (size_t i = 0; i < vec.size(); ++i) {
        s += std::to_string(vec[i]);
        if (i + 1 < vec.size()) {
            s += ", ";
        }
    }
    s += "}";
    return s;
}

/// @brief Join a vector of double values into a formatted string.
///
/// Output format:
/// `{v1, v2, v3}`
///
/// Intended for diagnostic message construction.
inline std::string joinNumbersDouble(const std::vector<double>& vec) {
    std::string s{"{"};
    for (size_t i = 0; i < vec.size(); ++i) {
        s += std::to_string(vec[i]);
        if (i + 1 < vec.size()) {
            s += ", ";
        }
    }
    s += "}";
    return s;
}

/// @brief Rethrow a concept exception with full compile-time metadata.
///
/// This macro captures:
///
/// - Concept name
/// - Variant name
/// - Encoding stage
/// - Section
///
/// and rethrows a `Mars2GribConceptException` while preserving
/// the nested exception chain.
#define MARS2GRIB_CONCEPT_RETHROW(CONCEPTNAME, MESSAGE)                                             \
    std::throw_with_nested(Mars2GribConceptException(std::string(CONCEPTNAME##Name),                \
                                                     std::string(CONCEPTNAME##TypeName<Variant>()), \
                                                     std::to_string(Stage), std::to_string(Section), MESSAGE, Here()))

/// @brief Throw a concept exception with full compile-time metadata.
///
/// Same as `MARS2GRIB_CONCEPT_RETHROW`, but without nesting.
/// Intended for initial throw sites.
#define MARS2GRIB_CONCEPT_THROW(CONCEPTNAME, MESSAGE)                                                                  \
    do {                                                                                                               \
        throw Mars2GribConceptException(std::string(CONCEPTNAME##Name), std::string(CONCEPTNAME##TypeName<Variant>()), \
                                        std::to_string(Stage), std::to_string(Section), MESSAGE, Here());              \
    } while (0)

}  // namespace metkit::mars2grib::utils::exceptions
