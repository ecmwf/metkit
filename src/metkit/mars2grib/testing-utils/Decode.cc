/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/Decode.h"

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "metkit/grib2mars/api/Grib2Mars.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::run_tests {

namespace {

using Recording = RecordingDictionary;

template <typename T>
const T* recorded(const Recording::Operation& operation) {
    const auto* typed = dynamic_cast<const Recording::TypedRecordedValue<T>*>(operation.value.get());
    return typed == nullptr ? nullptr : &typed->value();
}

void replaySet(metkit::codes::CodesHandle& handle, const Recording::Operation& operation) {
    using metkit::codes::Span;
    const std::string& key = operation.key;

    if (const auto* v = recorded<bool>(operation)) {
        handle.set(key, static_cast<long>(*v ? 1 : 0));
    }
    else if (const auto* v = recorded<long long>(operation)) {
        handle.set(key, static_cast<long>(*v));
    }
    else if (const auto* v = recorded<double>(operation)) {
        handle.set(key, *v);
    }
    else if (const auto* v = recorded<std::string>(operation)) {
        handle.set(key, *v);
    }
    else if (const auto* v = recorded<std::vector<long long>>(operation)) {
        const std::vector<long> values(v->begin(), v->end());
        handle.set(key, Span<const long>{values});
    }
    else if (const auto* v = recorded<std::vector<double>>(operation)) {
        handle.set(key, Span<const double>{*v});
    }
    else if (const auto* v = recorded<std::vector<std::string>>(operation)) {
        handle.set(key, Span<const std::string>{*v});
    }
    else if (const auto* v = recorded<std::vector<uint8_t>>(operation)) {
        handle.set(key, Span<const uint8_t>{*v});
    }
    else if (const auto* v = recorded<Recording::ValuesSummary>(operation)) {
        const std::vector<double> values(v->size, v->average);
        handle.set(key, Span<const double>{values});
    }
    else {
        throw eckit::BadValue("Cannot replay recorded value of key `" + key + "` (" + operation.value->category() + ")",
                              Here());
    }
}

//----------------------------------------------------------------------------------------------------------------------

std::string lower(std::string value) {
    for (auto& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

std::optional<double> asNumber(const std::string& text) {
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

std::string numberText(double value) {
    if (std::floor(value) == value && std::fabs(value) < 1e15) {
        return std::to_string(static_cast<long long>(value));
    }
    std::ostringstream os;
    os.precision(17);
    os << value;
    return os.str();
}

/// A configuration value as a list of normalised scalars, or nullopt for objects/mixed lists.
std::optional<std::vector<std::string>> scalars(const eckit::LocalConfiguration& c, const std::string& key) {
    if (c.isString(key)) {
        return std::vector<std::string>{c.getString(key)};
    }
    if (c.isBoolean(key)) {
        return std::vector<std::string>{c.getBool(key) ? "true" : "false"};
    }
    if (c.isIntegral(key)) {
        return std::vector<std::string>{std::to_string(c.getLong(key))};
    }
    if (c.isFloatingPoint(key)) {
        return std::vector<std::string>{numberText(c.getDouble(key))};
    }
    if (c.isStringList(key)) {
        return c.getStringVector(key);
    }
    if (c.isIntegralList(key)) {
        std::vector<std::string> out;
        for (long v : c.getLongVector(key)) {
            out.push_back(std::to_string(v));
        }
        return out;
    }
    if (c.isFloatingPointList(key)) {
        std::vector<std::string> out;
        for (double v : c.getDoubleVector(key)) {
            out.push_back(numberText(v));
        }
        return out;
    }
    return std::nullopt;
}

bool sameScalar(const std::string& a, const std::string& b) {
    if (lower(a) == lower(b)) {
        return true;
    }
    const auto x = asNumber(a);
    const auto y = asNumber(b);
    return x && y && *x == *y;
}

std::string text(const std::vector<std::string>& values) {
    std::string out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        out += (i == 0 ? "" : "/") + values[i];
    }
    return out;
}

std::string valueText(const eckit::LocalConfiguration& c, const std::string& key) {
    if (auto values = scalars(c, key)) {
        return text(*values);
    }
    return configurationToJson(c.getSubConfiguration(key));
}

void compareSection(const std::string& section, const eckit::LocalConfiguration& expected,
                    const eckit::LocalConfiguration& decoded, DecodeComparison& result) {
    for (const auto& key : expected.keys()) {
        std::string problem;
        if (!decoded.has(key)) {
            problem = "missing in decoded (expected " + valueText(expected, key) + ")";
        }
        else {
            const auto lhs = scalars(expected, key);
            const auto rhs = scalars(decoded, key);
            bool equal     = false;
            if (lhs && rhs) {
                equal = lhs->size() == rhs->size();
                for (std::size_t i = 0; equal && i < lhs->size(); ++i) {
                    equal = sameScalar((*lhs)[i], (*rhs)[i]);
                }
            }
            else if (!lhs && !rhs && expected.isSubConfiguration(key) && decoded.isSubConfiguration(key)) {
                equal = configurationToJson(expected.getSubConfiguration(key)) ==
                        configurationToJson(decoded.getSubConfiguration(key));
            }
            if (!equal) {
                problem = "expected " + valueText(expected, key) + ", decoded " + valueText(decoded, key);
            }
        }
        if (!problem.empty()) {
            result.equal = false;
            result.diff += "  " + section + "." + key + ": " + problem + "\n";
        }
    }
}

}  // namespace

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<metkit::codes::CodesHandle> replayOnCodesHandle(const RecordingDictionary& recording) {
    std::unique_ptr<metkit::codes::CodesHandle> handle;
    std::size_t index = 0;
    for (const auto& operation : recording.operations()) {
        try {
            switch (operation.kind) {
                case Recording::OperationKind::MakeFromSample: {
                    const auto* sample = recorded<Recording::SampleMetadata>(operation);
                    if (sample == nullptr) {
                        throw eckit::BadValue("make_from_sample without a sample name", Here());
                    }
                    handle = metkit::codes::codesHandleFromSample(sample->sample);
                    break;
                }
                case Recording::OperationKind::Clone:
                    if (!handle) {
                        throw eckit::BadValue("clone before make_from_sample", Here());
                    }
                    handle = handle->clone();
                    break;
                case Recording::OperationKind::SetMissing:
                    if (!handle) {
                        throw eckit::BadValue("set_missing before make_from_sample", Here());
                    }
                    handle->setMissing(operation.key);
                    break;
                case Recording::OperationKind::Set:
                    if (!handle) {
                        throw eckit::BadValue("set before make_from_sample", Here());
                    }
                    replaySet(*handle, operation);
                    break;
            }
        }
        catch (const std::exception& e) {
            throw eckit::Exception("Replaying operation " + std::to_string(index) + " " +
                                       recording.operation_to_json(operation) + " failed: " + e.what(),
                                   Here());
        }
        ++index;
    }
    if (!handle) {
        throw eckit::BadValue("Recorded set-list does not create a GRIB handle", Here());
    }
    return handle;
}

DecodedTestCase decodeRecording(const RecordingDictionary& recording, const eckit::LocalConfiguration& options) {
    const auto handle = replayOnCodesHandle(recording);

    eckit::LocalConfiguration grib2marsOptions;
    if (options.has("skipSection3") && options.isBoolean("skipSection3")) {
        grib2marsOptions.set("skipSection3", options.getBool("skipSection3"));
    }
    metkit::grib2mars::Grib2Mars grib2mars{grib2marsOptions};
    auto result = grib2mars.convert<eckit::LocalConfiguration>(*handle);
    return DecodedTestCase{std::move(result.mars), std::move(result.misc)};
}

DecodeComparison compareDecoded(const eckit::LocalConfiguration& expectedMars,
                                const eckit::LocalConfiguration& expectedMisc, const DecodedTestCase& decoded) {
    DecodeComparison result;
    compareSection("mars", expectedMars, decoded.mars, result);
    compareSection("misc", expectedMisc, decoded.misc, result);
    return result;
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
