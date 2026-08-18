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

/// @file Grib2MarsApiErrorHandling.h
/// @brief Common error handling for grib2mars public API calls.
#pragma once

#include <atomic>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include <unistd.h>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"

#include "metkit/grib2mars/api/Options.h"
#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::utils::exceptions {

namespace detail {

inline bool envVarEnablesStackMode(const char* name) {
    const char* value = std::getenv(name);

    if (value == nullptr) {
        return false;
    }

    std::string normalized;

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(value); *p != '\0'; ++p) {
        normalized.push_back(static_cast<char>(std::tolower(*p)));
    }

    return normalized == "1" || normalized == "true" || normalized == "on" || normalized == "yes";
}

/// @brief Sanitize a string for use in a generated filename.
inline std::string sanitizeForFileName(const std::string& value) {
    std::string out;
    out.reserve(value.size());

    for (const unsigned char c : value) {
        if (std::isalnum(c) || c == '_' || c == '-' || c == '.') {
            out.push_back(static_cast<char>(c));
        }
        else {
            out.push_back('_');
        }
    }

    return out;
}

/// @brief Convert the current thread identifier into a filename-safe string.
inline std::string currentThreadIdAsString() {
    std::ostringstream os;
    os << std::this_thread::get_id();

    return sanitizeForFileName(os.str());
}

/// @brief Resolve the directory where stack files are stored.
inline eckit::PathName errorStackDirectory(const metkit::grib2mars::Options& opts) {
    return eckit::PathName(opts.errorStackPath).fullName();
}

/// @brief Build a unique error-stack file path for an API entry point.
inline eckit::PathName makeErrorStackFilePath(const metkit::grib2mars::Options& opts, const std::string& apiName) {
    static std::atomic<unsigned long> counter{0};

    const eckit::PathName dir = errorStackDirectory(opts);

    try {
        dir.mkdir();
    }
    catch (const std::exception& e) {
        throw eckit::UserError("Unable to create Grib2Mars error stack directory `" + dir.asString() + "`: " + e.what(),
                               Here());
    }

    std::ostringstream fileName;

    fileName << "grib2mars-error-stack-" << sanitizeForFileName(apiName) << "-pid" << static_cast<long>(::getpid())
             << "-tid" << currentThreadIdAsString() << "-" << counter.fetch_add(1, std::memory_order_relaxed) << ".log";

    return dir / fileName.str();
}

/// @brief Format an eckit code location as file:line.
inline std::string formatCodeLocation(const eckit::CodeLocation& loc) {
    std::ostringstream os;
    os << loc.file() << ":" << loc.line();
    return os.str();
}

/// @brief Information extracted from the innermost exception.
struct InnermostExceptionInfo {
    std::string message;
    eckit::CodeLocation location;
};

/// @brief Extract the location from an eckit exception.
///
/// If the exception is not an eckit exception, fall back to the API boundary
/// location.
inline eckit::CodeLocation exceptionLocationOrFallback(const std::exception& exception,
                                                       const eckit::CodeLocation& fallback) {
    if (const auto* eckitException = dynamic_cast<const eckit::Exception*>(&exception)) {
        return eckitException->location();
    }

    return fallback;
}

/// @brief Recursively find the innermost std::nested_exception payload.
inline InnermostExceptionInfo innermostExceptionInfo(const std::exception& exception,
                                                     const eckit::CodeLocation& fallbackLocation) {
    const InnermostExceptionInfo current{
        exception.what(),
        exceptionLocationOrFallback(exception, fallbackLocation),
    };

    const auto* nested = dynamic_cast<const std::nested_exception*>(&exception);

    if (nested == nullptr) {
        return current;
    }

    const std::exception_ptr nestedPtr = nested->nested_ptr();

    if (!nestedPtr) {
        return current;
    }

    try {
        std::rethrow_exception(nestedPtr);
    }
    catch (const std::exception& nested) {
        return innermostExceptionInfo(nested, current.location);
    }
    catch (...) {
        return InnermostExceptionInfo{
            "<non-standard nested exception>",
            current.location,
        };
    }

    return current;
}

/// @brief Serialize a grib2mars exception stack to a stream.
inline void writeStructuredExceptionFrame(const std::exception& exception, std::ostream& out, std::size_t level,
                                          std::size_t frame) {
    const std::string pad = indent(level);

    out << pad << "+ " << std::string(lineSize, '=') << '\n'
        << pad << "+ frame " << frame << '\n'
        << pad << "+ " << std::string(lineSize, '-') << '\n';

    if (const auto* me = dynamic_cast<const Grib2MarsGenericException*>(&exception)) {
        me->printFrame(pad, out);
    }
    else {
        out << pad << "+ message: " << exception.what() << '\n';
    }

    out << pad << "+ " << std::string(lineSize, '+') << '\n';

    const auto* nested = dynamic_cast<const std::nested_exception*>(&exception);

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
    catch (const std::exception& nested) {
        writeStructuredExceptionFrame(nested, out, level + 1, frame + 1);
    }
    catch (...) {
        out << pad << "+ message: <non-standard nested exception>\n";
    }
}

/// @brief Write the structured API-boundary stack header to a stream.
inline void writeStructuredStackHeader(std::ostream& out, const std::string& apiName,
                                       const eckit::CodeLocation& loc        = eckit::CodeLocation(),
                                       const std::string* errorStackFilePath = nullptr) {
    out << "Grib2Mars API failure\n";
    out << "API entry point: " << apiName << "\n";
    if (errorStackFilePath != nullptr) {
        out << "Error stack file: " << *errorStackFilePath << "\n";
    }
    out << "\n";
    out << "Nested exception stack:\n";
    out << "-----------------------\n";

    out << "+ " << std::string(lineSize, '=') << '\n'
        << "+ frame 0\n"
        << "+ " << std::string(lineSize, '-') << '\n';

    out << "+ file:     " << loc.file() << "\n"
        << "+ function: " << loc.func() << "\n"
        << "+ line:     " << loc.line() << "\n"
        << "+ link:     " << loc.file() << ":" << loc.line() << "\n"
        << "+ message:  Error encountered at: " << apiName << "\n"
        << "+ " << std::string(lineSize, '+') << '\n';
}

/// @brief Serialize a grib2mars exception stack to disk.
inline void writeGrib2MarsExceptionStackToFile(const Grib2MarsGenericException& exception, const std::string& apiName,
                                               const eckit::PathName& errorStackFilePath,
                                               const eckit::CodeLocation& loc = eckit::CodeLocation()) {
    std::ofstream out(errorStackFilePath.asString());

    if (!out) {
        throw eckit::UserError("Grib2Mars conversion failed, but the error stack file could not be opened: `" +
                                   errorStackFilePath.asString() + "`",
                               Here());
    }

    const std::string path = errorStackFilePath.asString();
    writeStructuredStackHeader(out, apiName, loc, &path);
    writeStructuredExceptionFrame(exception, out, 1, 1);
}

/// @brief Print a grib2mars exception stack to standard error.
inline void writeGrib2MarsExceptionStackToStdErr(const Grib2MarsGenericException& exception, const std::string& apiName,
                                                 const eckit::CodeLocation& loc = eckit::CodeLocation()) {
    writeStructuredStackHeader(std::cerr, apiName, loc);
    writeStructuredExceptionFrame(exception, std::cerr, 1, 1);
}

/// @brief Build the short public-facing error message.
inline std::string buildPublicErrorMessage(const InnermostExceptionInfo& inner,
                                           const eckit::PathName& errorStackFilePath) {
    std::ostringstream os;

    os << "Grib2Mars conversion failed. "
       << "Innermost error: " << inner.message << " "
       << "(" << formatCodeLocation(inner.location) << "). "
       << "Full error stack written to `" << errorStackFilePath.asString() << "`";

    return os.str();
}

/// @brief Build the short public-facing error message when stack saving is disabled.
inline std::string buildPublicErrorMessage(const InnermostExceptionInfo& inner) {
    std::ostringstream os;

    os << "Grib2Mars conversion failed. "
       << "Innermost error: " << inner.message << " "
       << "(" << formatCodeLocation(inner.location) << ").";

    return os.str();
}

/// @brief Build the public-facing message when stack-file writing also failed.
inline std::string buildPublicErrorMessageWithoutStackFile(const InnermostExceptionInfo& inner,
                                                           const std::exception& stackWriteError) {
    std::ostringstream os;

    os << "Grib2Mars conversion failed. "
       << "Innermost error: " << inner.message << " "
       << "(" << formatCodeLocation(inner.location) << "). "
       << "Additionally, the full error stack could not be written: " << stackWriteError.what();

    return os.str();
}

}  // namespace detail

/// @brief Execute a callable under the grib2mars API error boundary.
///
/// @tparam Result
/// Callable return type.
///
/// @tparam Callable
/// Callable type.
///
/// @param[in] apiName
/// API entry-point name used in diagnostics.
///
/// @param[in] callable
/// Operation to execute.
///
/// @param[in] loc
/// Source location attached to user-facing failures.
template <typename Result, typename Callable>
Result withGrib2MarsApiErrorHandling(const std::string& apiName, const metkit::grib2mars::Options& opts,
                                     Callable&& callable,
                                     const eckit::CodeLocation& loc = eckit::CodeLocation()) {
    try {
        return std::forward<Callable>(callable)();
    }
    catch (const Grib2MarsGenericException& e) {
        const detail::InnermostExceptionInfo inner = detail::innermostExceptionInfo(e, loc);
        const bool printErrorStackToStdErr =
            opts.printErrorStackToStdErr || detail::envVarEnablesStackMode("GRIB2MARS_PRINT_STACK_TO_STDERR");
        const bool saveErrorStack =
            opts.saveErrorStack || detail::envVarEnablesStackMode("GRIB2MARS_WRITE_STACK_TO_FILE");

        if (printErrorStackToStdErr) {
            detail::writeGrib2MarsExceptionStackToStdErr(e, apiName, loc);
        }

        if (!saveErrorStack) {
            throw eckit::UserError(detail::buildPublicErrorMessage(inner), inner.location);
        }

        try {
            const eckit::PathName errorStackFilePath = detail::makeErrorStackFilePath(opts, apiName);

            detail::writeGrib2MarsExceptionStackToFile(e, apiName, errorStackFilePath, loc);

            throw eckit::UserError(detail::buildPublicErrorMessage(inner, errorStackFilePath), inner.location);
        }
        catch (const std::exception& stackWriteError) {
            throw eckit::UserError(detail::buildPublicErrorMessageWithoutStackFile(inner, stackWriteError),
                                   inner.location);
        }
    }
}

}  // namespace metkit::grib2mars::utils::exceptions
