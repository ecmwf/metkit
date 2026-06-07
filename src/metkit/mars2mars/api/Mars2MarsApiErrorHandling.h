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

/// @file Mars2MarsApiErrorHandling.h
/// @brief Common error handling for mars2mars public API calls.
#pragma once

#include <atomic>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#include <unistd.h>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"

#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::utils::exceptions {

namespace detail {

/// @brief Environment variable controlling error-stack output.
inline constexpr const char* MARS2MARS_ERROR_STACK_DIR_ENV = "MARS2MARS_ERROR_STACK_DIR";

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
inline eckit::PathName errorStackDirectory() {
    if (const char* value = std::getenv(MARS2MARS_ERROR_STACK_DIR_ENV)) {
        if (value[0] != '\0') {
            return eckit::PathName(value).fullName();
        }
    }

    return eckit::PathName(".").fullName();
}

/// @brief Build a unique error-stack file path for an API entry point.
inline eckit::PathName makeErrorStackFilePath(const std::string& apiName) {
    static std::atomic<unsigned long> counter{0};

    const eckit::PathName dir = errorStackDirectory();

    try {
        dir.mkdir();
    }
    catch (const std::exception& e) {
        throw eckit::UserError("Unable to create Mars2Mars error stack directory `" + dir.asString() + "`: " + e.what(),
                               Here());
    }

    std::ostringstream fileName;

    fileName << "mars2mars-error-stack-" << sanitizeForFileName(apiName) << "-pid" << static_cast<long>(::getpid())
             << "-tid" << currentThreadIdAsString() << "-" << counter.fetch_add(1, std::memory_order_relaxed) << ".log";

    return dir / fileName.str();
}

/// @brief Serialize a mars2mars exception stack to disk.
inline void writeMars2MarsExceptionStackToFile(const Mars2marsGenericException& exception, const std::string& apiName,
                                                const eckit::PathName& errorStackFilePath,
                                                const eckit::CodeLocation& loc = eckit::CodeLocation()) {

    std::ofstream out(errorStackFilePath.asString());

    if (!out) {
        throw eckit::UserError(
            "Mars2Mars conversion failed, but the error stack file could not "
            "be opened: `" +
                errorStackFilePath.asString() + "`",
            Here());
    }

    out << "Mars2Mars API failure\n";
    out << "API entry point: " << apiName << "\n";
    out << "Error stack file: " << errorStackFilePath.asString() << "\n";
    out << "\n";
    out << "Nested exception stack:\n";
    out << "-----------------------\n";

    out << "+ " << std::string(lineSize, '=') << std::endl
        << "+ frame " << 0 << std::endl
        << "+ " << std::string(lineSize, '-') << std::endl;
    out << "+ file:     " << loc.file() << "\n"
        << "+ function: " << loc.func() << "\n"
        << "+ line:     " << loc.line() << "\n"
        << "+ link:     " << loc.file() << ":" << loc.line() << "\n"
        << "+ message:  " << "Error encountered at: " << apiName << "\n";
    out << "+ " << std::string(lineSize, '+') << std::endl;

    printExtendedStack(exception, out, 1, 1);
}

}  // namespace detail

/// @brief Execute a callable under the mars2mars API error boundary.
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
Result withMars2MarsApiErrorHandling(const std::string& apiName, Callable&& callable,
                                     const eckit::CodeLocation& loc = eckit::CodeLocation()) {

    try {
        return std::forward<Callable>(callable)();
    }
    catch (const Mars2marsGenericException& e) {
        const eckit::PathName errorStackFilePath = detail::makeErrorStackFilePath(apiName);

        std::cout << "Mars2Mars API error" << std::endl;
        detail::writeMars2MarsExceptionStackToFile(e, apiName, errorStackFilePath, loc);

        throw eckit::UserError(
            "Mars2Mars conversion failed. Full error stack written to `" + errorStackFilePath.asString() + "`", loc);
    }
    catch (...) {
        throw;
    }
}

}  // namespace metkit::mars2mars::utils::exceptions
