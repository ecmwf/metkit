/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/TestRecord.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <optional>
#include <sstream>

#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/JSON.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/value/Value.h"

namespace metkit::mars2grib::testing_utils::run_tests {

namespace {

/// LocalConfiguration built from a parsed value (that constructor of LocalConfiguration is protected).
class JsonConfiguration : public eckit::LocalConfiguration {
public:

    explicit JsonConfiguration(const eckit::Value& value) : eckit::LocalConfiguration(value) {}
};

}  // namespace

TestMode testModeFromString(const std::string& name) {
    if (name == "encode") {
        return TestMode::Encode;
    }
    if (name == "decode") {
        return TestMode::Decode;
    }
    if (name == "roundtrip") {
        return TestMode::Roundtrip;
    }
    throw eckit::UserError("Unknown test mode `" + name + "` (encode, decode, roundtrip)", Here());
}

std::string toString(TestMode mode) {
    switch (mode) {
        case TestMode::Encode:
            return "encode";
        case TestMode::Decode:
            return "decode";
        case TestMode::Roundtrip:
            return "roundtrip";
    }
    return "unknown";
}

eckit::LocalConfiguration parseRecord(std::string_view json, RecordParser parser) {
    if (parser == RecordParser::Yaml) {
        return eckit::LocalConfiguration{eckit::YAMLConfiguration{std::string{json}}};
    }

    const eckit::Value root = eckit::JSONParser::decodeString(std::string{json});
    if (!root.isMap()) {
        throw eckit::UserError("Test-case record must be a JSON object", Here());
    }
    return JsonConfiguration{root};
}

eckit::LocalConfiguration requiredSubConfiguration(const eckit::LocalConfiguration& root, const std::string& key) {
    if (!root.has(key) || !root.isSubConfiguration(key)) {
        throw eckit::UserError("Test-case record requires object `" + key + "`", Here());
    }
    return root.getSubConfiguration(key);
}

std::optional<eckit::LocalConfiguration> optionalSubConfiguration(const eckit::LocalConfiguration& root,
                                                                  const std::string& key) {
    if (!root.has(key) || !root.isSubConfiguration(key)) {
        return std::nullopt;
    }
    return root.getSubConfiguration(key);
}

namespace {

std::string lowercase(std::string value) {
    for (auto& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

std::optional<double> number(const std::string& text) {
    if (text.empty()) {
        return std::nullopt;
    }
    errno              = 0;
    char* end          = nullptr;
    const double value = std::strtod(text.c_str(), &end);
    if (*end != '\0' || errno == ERANGE) {
        return std::nullopt;
    }
    return value;
}

std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r");
    if (first == std::string::npos) {
        return {};
    }
    return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
}

/// The value of `key` as text, one entry per list element.
std::vector<std::string> marsValues(const eckit::LocalConfiguration& mars, const std::string& key) {
    if (mars.isString(key)) {
        return {mars.getString(key)};
    }
    if (mars.isBoolean(key)) {
        return {mars.getBool(key) ? "true" : "false"};
    }
    if (mars.isIntegral(key)) {
        return {std::to_string(mars.getLong(key))};
    }
    if (mars.isFloatingPoint(key)) {
        std::ostringstream os;
        os.precision(17);
        os << mars.getDouble(key);
        return {os.str()};
    }
    if (mars.isStringList(key)) {
        return mars.getStringVector(key);
    }
    if (mars.isIntegralList(key)) {
        std::vector<std::string> out;
        for (long v : mars.getLongVector(key)) {
            out.push_back(std::to_string(v));
        }
        return out;
    }
    if (mars.isFloatingPointList(key)) {
        std::vector<std::string> out;
        for (double v : mars.getDoubleVector(key)) {
            std::ostringstream os;
            os.precision(17);
            os << v;
            out.push_back(os.str());
        }
        return out;
    }
    return {};
}

}  // namespace

void MarsRecordFilter::addRules(const std::string& text) {
    std::string normalised = text;
    for (auto& c : normalised) {
        if (c == ';') {
            c = '\n';
        }
    }
    std::istringstream lines(normalised);
    std::string line;
    while (std::getline(lines, line)) {
        line = trim(line.substr(0, line.find('#')));
        if (line.empty()) {
            continue;
        }

        std::vector<Condition> rule;
        std::vector<std::string> terms;
        eckit::Tokenizer(",")(line, terms);
        for (const auto& rawTerm : terms) {
            const std::string term = trim(rawTerm);
            const auto equal       = term.find('=');
            if (equal == std::string::npos || equal == 0 || equal + 1 == term.size()) {
                throw eckit::UserError("Invalid MARS filter term `" + term + "`, expected key=value[/value...]",
                                       Here());
            }
            Condition condition;
            condition.key = lowercase(trim(term.substr(0, equal)));
            std::vector<std::string> values;
            eckit::Tokenizer("/")(term.substr(equal + 1), values);
            for (std::size_t i = 0; i < values.size(); ++i) {
                const std::string value = trim(values[i]);
                if (i + 2 < values.size() && lowercase(trim(values[i + 1])) == "to") {
                    const auto from = number(value);
                    const auto to   = number(trim(values[i + 2]));
                    if (!from || !to) {
                        throw eckit::UserError("Range `" + term + "` needs numeric bounds", Here());
                    }
                    condition.ranges.emplace_back(*from, *to);
                    i += 2;
                }
                else if (lowercase(value) == "missing") {
                    condition.missing = true;
                }
                else {
                    condition.values.push_back(value);
                }
            }
            rule.push_back(std::move(condition));
        }
        rules_.push_back(std::move(rule));
    }
}

bool MarsRecordFilter::matches(const eckit::LocalConfiguration& mars) const {
    const auto conditionMatches = [&](const Condition& condition) {
        if (!mars.has(condition.key)) {
            return condition.missing;
        }
        for (const auto& actual : marsValues(mars, condition.key)) {
            const auto actualNumber = number(actual);
            for (const auto& value : condition.values) {
                if (lowercase(actual) == lowercase(value)) {
                    return true;
                }
                const auto valueNumber = number(value);
                if (actualNumber && valueNumber && *actualNumber == *valueNumber) {
                    return true;
                }
            }
            for (const auto& [from, to] : condition.ranges) {
                if (actualNumber && from <= *actualNumber && *actualNumber <= to) {
                    return true;
                }
            }
        }
        return false;
    };

    for (const auto& rule : rules_) {
        bool all = true;
        for (const auto& condition : rule) {
            if (!conditionMatches(condition)) {
                all = false;
                break;
            }
        }
        if (all) {
            return true;
        }
    }
    return false;
}

std::vector<std::size_t> parseBlockIds(const std::string& expression, std::size_t numberOfBlocks) {
    std::vector<std::size_t> ids;
    std::vector<std::string> terms;
    eckit::Tokenizer(",")(expression, terms);
    for (const auto& term : terms) {
        const auto dash   = term.find('-');
        std::size_t first = 0;
        std::size_t last  = 0;
        try {
            first = std::stoul(term.substr(0, dash));
            last  = dash == std::string::npos ? first : std::stoul(term.substr(dash + 1));
        }
        catch (const std::exception&) {
            throw eckit::UserError("Invalid block id `" + term + "`, expected e.g. 0,3,5-7", Here());
        }
        if (first > last || last >= numberOfBlocks) {
            throw eckit::UserError(
                "Block range `" + term + "` is outside the " + std::to_string(numberOfBlocks) + " blocks of the file",
                Here());
        }
        for (std::size_t id = first; id <= last; ++id) {
            ids.push_back(id);
        }
    }
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    return ids;
}

std::string configurationToJson(const eckit::LocalConfiguration& configuration) {
    std::ostringstream os;
    os.precision(17);  // enough digits to read every double back exactly
    eckit::JSON json(os);
    json << configuration;
    return os.str();
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
