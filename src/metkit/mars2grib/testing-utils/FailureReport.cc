/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/FailureReport.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <utility>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"

#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::run_tests {

namespace detail {

namespace {

bool isPlaceholderCharacter(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.';
}

/// Index of the `]` closing the array opened at `open`, or npos if the array contains nested
/// arrays or objects (those are printed one element per line).
std::size_t flatArrayEnd(std::string_view json, std::size_t open) {
    bool inString = false;
    bool escape   = false;
    for (std::size_t i = open + 1; i < json.size(); ++i) {
        const char c = json[i];
        if (inString) {
            if (escape) {
                escape = false;
            }
            else if (c == '\\') {
                escape = true;
            }
            else if (c == '"') {
                inString = false;
            }
            continue;
        }
        if (c == '"') {
            inString = true;
        }
        else if (c == '[' || c == '{') {
            return std::string_view::npos;
        }
        else if (c == ']') {
            return i;
        }
    }
    return std::string_view::npos;
}

std::string compactFlatArray(std::string_view json) {
    std::string out;
    bool inString = false;
    bool escape   = false;
    for (const char c : json) {
        if (inString) {
            out += c;
            if (escape) {
                escape = false;
            }
            else if (c == '\\') {
                escape = true;
            }
            else if (c == '"') {
                inString = false;
            }
            continue;
        }
        if (c == '"') {
            inString = true;
            out += c;
        }
        else if (c == ',') {
            out += ", ";
        }
        else if (!std::isspace(static_cast<unsigned char>(c))) {
            out += c;
        }
    }
    return out;
}

using OperationKind = RecordingDictionary::OperationKind;

struct KeyValue {
    std::string datatype;
    std::string value;
};

std::map<std::string, KeyValue> finalKeyValues(const RecordingDictionary& dictionary) {
    std::map<std::string, KeyValue> values;
    for (const auto& operation : dictionary.operations()) {
        if (operation.kind == OperationKind::Set && operation.value) {
            values[operation.key] = KeyValue{operation.value->datatype_json(), operation.value->value_json()};
        }
        else if (operation.kind == OperationKind::SetMissing) {
            values[operation.key] = KeyValue{"missing", "MISSING"};
        }
    }
    return values;
}

std::string keyDiff(const RecordingDictionary& expected, const RecordingDictionary& actual) {
    const auto lhs = finalKeyValues(expected);
    const auto rhs = finalKeyValues(actual);

    std::ostringstream os;
    std::size_t differences = 0;
    auto l                  = lhs.begin();
    auto r                  = rhs.begin();
    while (l != lhs.end() || r != rhs.end()) {
        if (r == rhs.end() || (l != lhs.end() && l->first < r->first)) {
            os << "  - " << l->first << " (only in expected): " << l->second.value << '\n';
            ++differences;
            ++l;
        }
        else if (l == lhs.end() || r->first < l->first) {
            os << "  + " << r->first << " (only in current): " << r->second.value << '\n';
            ++differences;
            ++r;
        }
        else {
            if (l->second.value != r->second.value || l->second.datatype != r->second.datatype) {
                os << "  ~ " << l->first << '\n';
                os << "      expected: " << l->second.value;
                if (l->second.datatype != r->second.datatype) {
                    os << "  " << l->second.datatype;
                }
                os << '\n' << "      current : " << r->second.value;
                if (l->second.datatype != r->second.datatype) {
                    os << "  " << r->second.datatype;
                }
                os << '\n';
                ++differences;
            }
            ++l;
            ++r;
        }
    }

    if (differences == 0) {
        return "  (final key values are identical; operations differ in order, samples or clones)\n";
    }
    return os.str();
}

std::string formatIndex(std::size_t index) {
    std::ostringstream os;
    os << '[' << std::setw(4) << index << ']';
    return os.str();
}

std::string operationDiff(const RecordingDictionary& expected, const RecordingDictionary& actual) {
    std::vector<std::string> lhs;
    std::vector<std::string> rhs;
    lhs.reserve(expected.operation_count());
    rhs.reserve(actual.operation_count());
    for (const auto& operation : expected.operations()) {
        lhs.push_back(expected.operation_to_json(operation));
    }
    for (const auto& operation : actual.operations()) {
        rhs.push_back(actual.operation_to_json(operation));
    }

    // Strip the common prefix and suffix, then align the rest with a longest common subsequence.
    std::size_t prefix = 0;
    while (prefix < lhs.size() && prefix < rhs.size() && lhs[prefix] == rhs[prefix]) {
        ++prefix;
    }
    std::size_t suffix = 0;
    while (suffix < lhs.size() - prefix && suffix < rhs.size() - prefix &&
           lhs[lhs.size() - 1 - suffix] == rhs[rhs.size() - 1 - suffix]) {
        ++suffix;
    }

    const std::size_t n = lhs.size() - prefix - suffix;
    const std::size_t m = rhs.size() - prefix - suffix;

    std::ostringstream os;
    const auto removed = [&](std::size_t i) { os << "  - " << formatIndex(i) << ' ' << lhs[i] << '\n'; };
    const auto added   = [&](std::size_t j) { os << "  + " << formatIndex(j) << ' ' << rhs[j] << '\n'; };

    constexpr std::size_t maxTableSize = 4'000'000;
    if (n * m > maxTableSize) {
        // Too large to align: list the differing range positionally.
        for (std::size_t k = 0; k < n; ++k) {
            removed(prefix + k);
        }
        for (std::size_t k = 0; k < m; ++k) {
            added(prefix + k);
        }
        return os.str();
    }

    // table[i][j] = LCS length of lhs[prefix+i..] and rhs[prefix+j..]
    std::vector<std::uint32_t> table((n + 1) * (m + 1), 0);
    const auto at = [&](std::size_t i, std::size_t j) -> std::uint32_t& { return table[i * (m + 1) + j]; };
    for (std::size_t i = n; i-- > 0;) {
        for (std::size_t j = m; j-- > 0;) {
            at(i, j) = lhs[prefix + i] == rhs[prefix + j] ? at(i + 1, j + 1) + 1 : std::max(at(i + 1, j), at(i, j + 1));
        }
    }

    std::size_t i = 0;
    std::size_t j = 0;
    while (i < n || j < m) {
        if (i < n && j < m && lhs[prefix + i] == rhs[prefix + j]) {
            ++i;
            ++j;
        }
        else if (j == m || (i < n && at(i + 1, j) >= at(i, j + 1))) {
            removed(prefix + i);
            ++i;
        }
        else {
            added(prefix + j);
            ++j;
        }
    }
    return os.str();
}

std::string sanitizePathComponent(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (const char c : value) {
        out += isPlaceholderCharacter(c) ? c : '_';
    }
    if (out.empty()) {
        return "none";
    }
    if (out == "." || out == "..") {
        return std::string(out.size(), '_');
    }
    return out;
}

std::string marsValue(const eckit::LocalConfiguration* mars, const std::string& key) {
    if (mars == nullptr || !mars->has(key)) {
        return "none";
    }

    const auto join = [](const auto& values) {
        std::ostringstream os;
        for (std::size_t i = 0; i < values.size(); ++i) {
            os << (i == 0 ? "" : "_") << values[i];
        }
        return os.str();
    };

    if (mars->isString(key)) {
        return mars->getString(key);
    }
    if (mars->isBoolean(key)) {
        return mars->getBool(key) ? "true" : "false";
    }
    if (mars->isIntegral(key)) {
        return std::to_string(mars->getLong(key));
    }
    if (mars->isFloatingPoint(key)) {
        std::ostringstream os;
        os << mars->getDouble(key);
        return os.str();
    }
    if (mars->isStringList(key)) {
        return join(mars->getStringVector(key));
    }
    if (mars->isIntegralList(key)) {
        return join(mars->getLongVector(key));
    }
    return "complex";
}

void writeSection(std::ostream& out, const std::string& title) {
    std::string line = "==== " + title + " ";
    if (line.size() < 80) {
        line.append(80 - line.size(), '=');
    }
    out << '\n' << line << '\n';
}

void writeConfiguration(std::ostream& out, const eckit::LocalConfiguration* configuration) {
    if (configuration == nullptr) {
        out << "(unavailable)\n";
        return;
    }
    out << prettyJson(configurationToJson(*configuration)) << '\n';
}

}  // namespace

std::string prettyJson(std::string_view json) {
    std::string out;
    out.reserve(json.size() * 2);
    std::size_t indent = 0;
    bool inString      = false;
    bool escape        = false;

    const auto newline = [&]() {
        out += '\n';
        out.append(2 * indent, ' ');
    };

    for (std::size_t i = 0; i < json.size(); ++i) {
        const char c = json[i];
        if (inString) {
            out += c;
            if (escape) {
                escape = false;
            }
            else if (c == '\\') {
                escape = true;
            }
            else if (c == '"') {
                inString = false;
            }
            continue;
        }

        switch (c) {
            case '"':
                inString = true;
                out += c;
                break;
            case '[': {
                const std::size_t end = flatArrayEnd(json, i);
                if (end != std::string_view::npos) {
                    out += compactFlatArray(json.substr(i, end - i + 1));
                    i = end;
                    break;
                }
                [[fallthrough]];
            }
            case '{': {
                std::size_t next = i + 1;
                while (next < json.size() && std::isspace(static_cast<unsigned char>(json[next]))) {
                    ++next;
                }
                if (next < json.size() && (json[next] == '}' || json[next] == ']')) {
                    out += c;
                    out += json[next];
                    i = next;
                    break;
                }
                out += c;
                ++indent;
                newline();
                break;
            }
            case '}':
            case ']':
                if (indent > 0) {
                    --indent;
                }
                newline();
                out += c;
                break;
            case ',':
                out += c;
                newline();
                break;
            case ':':
                out += ": ";
                break;
            default:
                if (!std::isspace(static_cast<unsigned char>(c))) {
                    out += c;
                }
                break;
        }
    }
    return out;
}

std::string formatOperations(const RecordingDictionary& dictionary) {
    std::ostringstream os;
    os << dictionary.operation_count() << " operations\n";
    std::size_t index = 0;
    for (const auto& operation : dictionary.operations()) {
        os << formatIndex(index++) << ' ' << dictionary.operation_to_json(operation) << '\n';
    }
    return os.str();
}

std::string diffOutputs(const RecordingDictionary& expected, const RecordingDictionary& actual) {
    std::ostringstream os;
    const auto first = expected.compare(actual);
    if (first.equal) {
        return "(outputs are identical)\n";
    }
    os << "First mismatch: " << first.path << ": " << first.reason << '\n';
    os << "\nKeys (final value of each key; - only in expected, + only in current, ~ changed):\n";
    os << keyDiff(expected, actual);
    os << "\nOperations (- expected, + current; index into the respective list):\n";
    os << operationDiff(expected, actual);
    return os.str();
}

std::string recordHash(std::string_view record) {
    // 64-bit FNV-1a: stable across runs and platforms.
    std::uint64_t hash = 14695981039346656037ULL;
    for (const char c : record) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }
    char buffer[17];
    std::snprintf(buffer, sizeof(buffer), "%016llx", static_cast<unsigned long long>(hash));
    return buffer;
}

}  // namespace detail

FailureReporter::FailureReporter(FailureReportOptions options) :
    options_{std::move(options)}, tokens_{tokenize(options_.schema)} {
    validateSchema(options_.schema);
    if (options_.directory.empty()) {
        throw eckit::UserError("Failure report directory must not be empty", Here());
    }
    while (options_.directory.size() > 1 && options_.directory.back() == '/') {
        options_.directory.pop_back();
    }
}

std::vector<FailureReporter::Token> FailureReporter::tokenize(const std::string& schema) {
    std::vector<Token> tokens;
    std::string literal;
    for (std::size_t i = 0; i < schema.size(); ++i) {
        const char c = schema[i];
        if (c == '}') {
            throw eckit::UserError("Failure report schema `" + schema + "` has an unmatched `}`", Here());
        }
        if (c != '{') {
            literal += c;
            continue;
        }

        const std::size_t close = schema.find('}', i + 1);
        if (close == std::string::npos) {
            throw eckit::UserError("Failure report schema `" + schema + "` has an unmatched `{`", Here());
        }
        const std::string name = schema.substr(i + 1, close - i - 1);
        if (name.empty()) {
            throw eckit::UserError("Failure report schema `" + schema + "` has an empty `{}`", Here());
        }
        for (const char n : name) {
            if (!detail::isPlaceholderCharacter(n)) {
                throw eckit::UserError("Failure report schema `" + schema + "`: invalid placeholder `{" + name + "}`",
                                       Here());
            }
        }

        if (!literal.empty()) {
            tokens.push_back(Token{false, std::move(literal)});
            literal.clear();
        }
        tokens.push_back(Token{true, name});
        i = close;
    }
    if (!literal.empty()) {
        tokens.push_back(Token{false, std::move(literal)});
    }
    return tokens;
}

void FailureReporter::validateSchema(const std::string& schema) {
    if (schema.empty()) {
        throw eckit::UserError("Failure report schema must not be empty", Here());
    }
    if (schema.front() == '/') {
        throw eckit::UserError("Failure report schema `" + schema + "` must be a relative path", Here());
    }
    if (schema.back() == '/') {
        throw eckit::UserError("Failure report schema `" + schema + "` must end with a file name", Here());
    }

    tokenize(schema);

    std::size_t start = 0;
    while (start <= schema.size()) {
        const std::size_t end       = std::min(schema.find('/', start), schema.size());
        const std::string component = schema.substr(start, end - start);
        if (component.empty() || component == "." || component == "..") {
            throw eckit::UserError(
                "Failure report schema `" + schema + "` must not contain empty, `.` or `..` path components", Here());
        }
        start = end + 1;
    }
}

std::string FailureReporter::resolvePath(const FailureDetails& details) const {
    std::string path;
    for (const auto& token : tokens_) {
        if (!token.placeholder) {
            path += token.text;
        }
        else if (token.text == "hash") {
            path += detail::recordHash(details.record);
        }
        else if (token.text == "status") {
            path += details.error ? "error" : "mismatch";
        }
        else {
            path += detail::sanitizePathComponent(detail::marsValue(details.mars, token.text));
        }
    }
    return options_.directory + "/" + path;
}

void FailureReporter::ensureDirectory(const std::string& directory) {
    if (createdDirectories_.count(directory) != 0) {
        return;
    }
    const eckit::PathName path{directory};
    try {
        path.mkdir();
    }
    catch (const eckit::Exception&) {
        // Another thread may have created it concurrently.
        if (!path.exists() || !path.isDir()) {
            throw;
        }
    }
    createdDirectories_.insert(directory);
}

std::string FailureReporter::write(const FailureDetails& details) {
    const std::string path = resolvePath(details);
    ensureDirectory(path.substr(0, path.rfind('/')));

    std::ofstream out{path, std::ios::out | std::ios::trunc};
    if (!out) {
        throw eckit::Exception("Unable to open failure report `" + path + "`", Here());
    }

    const bool encoded = details.mode != TestMode::Decode;
    const bool decoded = details.mode != TestMode::Encode;

    out << "mars2grib failed test\n";
    out << "mode: " << toString(details.mode) << '\n';
    out << "status: " << (details.error ? "ERROR (exception while running the test)" : "MISMATCH") << '\n';
    out << "reason: " << details.reason << '\n';

    detail::writeSection(out, "MARS");
    detail::writeConfiguration(out, details.mars);
    detail::writeSection(out, "MISC");
    detail::writeConfiguration(out, details.misc);
    detail::writeSection(out, "OPT");
    detail::writeConfiguration(out, details.options);

    detail::writeSection(out, "EXPECTED OUTPUT");
    out << (details.expected ? detail::formatOperations(*details.expected) : "(unavailable)\n");

    if (encoded) {
        detail::writeSection(out, "CURRENT OUTPUT");
        if (details.actual) {
            out << detail::formatOperations(*details.actual);
        }
        else {
            out << "(unavailable: no output was produced)\n" << details.reason << '\n';
        }

        detail::writeSection(out, "DIFF");
        if (details.expected && details.actual) {
            out << detail::diffOutputs(*details.expected, *details.actual);
        }
        else {
            out << "(unavailable: " << (details.expected ? "current" : "expected") << " output missing)\n";
        }
    }

    if (decoded) {
        detail::writeSection(out, details.mode == TestMode::Roundtrip ? "DECODED MARS (from the current output)"
                                                                      : "DECODED MARS (from the expected output)");
        detail::writeConfiguration(out, details.decodedMars);
        detail::writeSection(out, "DECODED MISC");
        detail::writeConfiguration(out, details.decodedMisc);
        detail::writeSection(out, "DECODE DIFF (keys of the input mars/misc)");
        if (details.decodedMars == nullptr) {
            out << "(unavailable: decoding did not run or failed)\n";
        }
        else if (details.decodeDiff.empty()) {
            out << "(decoded mars and misc match the input)\n";
        }
        else {
            out << details.decodeDiff;
        }
    }

    detail::writeSection(out, "RECORD");
    out << details.record << '\n';

    out.close();
    if (!out) {
        throw eckit::Exception("Unable to write failure report `" + path + "`", Here());
    }
    return path;
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
