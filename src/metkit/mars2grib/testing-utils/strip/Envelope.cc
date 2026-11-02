/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/strip/Envelope.h"

#include <cstring>
#include <utility>

#include "eckit/exception/Exceptions.h"

#include "metkit/datamod/Json.h"
#include "metkit/mars2grib/api/Mars2GribClassify.h"
#include "metkit/mars2grib/backend/deductions/step.h"
#include "metkit/mars2grib/testing-utils/TestCaseLabels.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"

namespace metkit::mars2grib::testing_utils::strip {

namespace {

using run_tests::parseRecord;
using run_tests::RecordParser;
using run_tests::topLevelMembers;

std::string_view member(std::string_view record,
                        const std::map<std::string, std::pair<std::size_t, std::size_t>>& members,
                        const std::string& key) {
    const auto it = members.find(key);
    if (it == members.end()) {
        return {};
    }
    return record.substr(it->second.first, it->second.second - it->second.first);
}

std::string text(const eckit::LocalConfiguration& mars, const std::string& key) {
    if (mars.has(key) && mars.isString(key)) {
        return mars.getString(key);
    }
    if (mars.has(key) && mars.isIntegral(key)) {
        return std::to_string(mars.getLong(key));
    }
    return missingValue;
}

void putString(std::vector<char>& out, const std::string& value) {
    const std::uint64_t size = value.size();
    const char* bytes        = reinterpret_cast<const char*>(&size);
    out.insert(out.end(), bytes, bytes + sizeof(size));
    out.insert(out.end(), value.begin(), value.end());
}

bool getString(const std::vector<char>& in, std::size_t& pos, std::string& value) {
    std::uint64_t size = 0;
    if (in.size() - pos < sizeof(size)) {
        return false;
    }
    std::memcpy(&size, in.data() + pos, sizeof(size));
    pos += sizeof(size);
    if (in.size() - pos < size) {
        throw eckit::SeriousBug("Truncated envelope in stripping exchange buffer", Here());
    }
    value.assign(in.data() + pos, size);
    pos += size;
    return true;
}

}  // namespace

std::string Envelope::timeRepresentation() const {
    return anchorType + "/" + domainType + "/" + shapeType;
}

Envelope makeEnvelope(std::string record) {
    Envelope envelope;
    envelope.record = std::move(record);

    const std::string_view view = envelope.record;
    const auto members          = topLevelMembers(view);
    const auto section          = [&](const std::string& key) {
        const std::string_view json = member(view, members, key);
        return json.empty() ? eckit::LocalConfiguration{} : parseRecord(json, RecordParser::Json);
    };
    const eckit::LocalConfiguration mars = section("mars");
    const eckit::LocalConfiguration misc = section("misc");
    const eckit::LocalConfiguration opt  = section("opt");

    try {
        metkit::mars2grib::Mars2GribClassify classifier{opt};
        const auto spec  = classifier.computeProductTimeSpec(mars, misc);
        const auto json  = metkit::datamod::parseJsonObject(spec.json);
        const auto field = [&](const char* key) {
            const metkit::datamod::Value* value = json.find(key);
            return value != nullptr && value->isString() ? value->asString() : std::string{missingValue};
        };
        envelope.anchorType = field("anchorType");
        envelope.domainType = field("domainType");
        envelope.shapeType  = field("shapeType");

        // Same rule as mars2grib-prune-tests-otr.
        const auto step        = metkit::mars2grib::backend::deductions::resolve_Step_opt(mars, misc, opt);
        envelope.firstEmission = !step.has_value() || step->length == 0 ||
                                 step->length == classifier.computeOuterTimeRangeInHours(mars, misc);
        envelope.classified = true;
    }
    catch (...) {
        // Unclassifiable records keep `missing` and are not dropped by first-emission.
    }

    std::uint64_t hash = run_tests::fnv1a("route");
    for (const std::string& part :
         {text(mars, "class"), text(mars, "stream"), text(mars, "type"), text(mars, "levtype"), envelope.anchorType,
          envelope.domainType, envelope.shapeType}) {
        hash = run_tests::fnv1a("\x1F", run_tests::fnv1a(part, hash));
    }
    envelope.route = hash;
    return envelope;
}

void appendEnvelope(std::vector<char>& buffer, const Envelope& envelope) {
    putString(buffer, envelope.record);
    putString(buffer, envelope.anchorType);
    putString(buffer, envelope.domainType);
    putString(buffer, envelope.shapeType);
    const char* route = reinterpret_cast<const char*>(&envelope.route);
    buffer.insert(buffer.end(), route, route + sizeof(envelope.route));
    buffer.push_back(static_cast<char>((envelope.firstEmission ? 1 : 0) | (envelope.classified ? 2 : 0)));
}

bool readEnvelope(const std::vector<char>& buffer, std::size_t& position, Envelope& envelope) {
    if (!getString(buffer, position, envelope.record)) {
        return false;
    }
    if (!getString(buffer, position, envelope.anchorType) || !getString(buffer, position, envelope.domainType) ||
        !getString(buffer, position, envelope.shapeType) || buffer.size() - position < sizeof(envelope.route) + 1) {
        throw eckit::SeriousBug("Truncated envelope in stripping exchange buffer", Here());
    }
    std::memcpy(&envelope.route, buffer.data() + position, sizeof(envelope.route));
    position += sizeof(envelope.route);
    const char flags       = buffer[position++];
    envelope.firstEmission = (flags & 1) != 0;
    envelope.classified    = (flags & 2) != 0;
    return true;
}

TestView TestView::parse(const Envelope& envelope) {
    const std::string_view view = envelope.record;
    const auto members          = topLevelMembers(view);
    const auto section          = [&](const std::string& key) {
        const std::string_view json = member(view, members, key);
        return json.empty() ? metkit::datamod::Dict{} : metkit::datamod::parseJsonObject(json);
    };
    return TestView{envelope, section("mars"), section("misc"), section("opt")};
}

}  // namespace metkit::mars2grib::testing_utils::strip
