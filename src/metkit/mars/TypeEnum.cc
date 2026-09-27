/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/TypeEnum.h"

#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Serialize.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

void TypeEnum::addValue(const std::string& vv, uint16_t idx, bool allowDuplicates) const {
    std::string value = eckit::StringTools::lower(vv);
    if (!allowDuplicates && values_.find(value) != values_.end()) {
        std::ostringstream oss;
        oss << "Redefined enum value '" << value << "' while parsing " << name();
        throw eckit::SeriousBug(oss.str());
    }
    // skip descriptions: and strings too long or containing blanks
    if (value.size() < 20 && value.find(" ") == std::string::npos) {
        values_[value] = idx;
    }
}

uint16_t TypeEnum::parseValueNames(const eckit::Value& names, bool allowDuplicates) const {
    std::string firstName = names.isList() ? names[0] : names;
    uint16_t idx          = groups_.size();
    addValue(firstName, idx, allowDuplicates);
    if (names.isList()) {
        for (size_t i = 1; i < names.size(); ++i) {
            addValue(names[i], idx, allowDuplicates);
        }
    }
    if (flags_[3]) {
        firstName = eckit::StringTools::upper(firstName);
    }
    groups_.emplace_back(firstName, std::vector<std::string>{});
    return idx;
}

std::vector<std::string> TypeEnum::parseEnumValue(const eckit::Value& val, bool allowDuplicates) const {

    if (val.isMap()) {
        flags_[5] = true;

        ASSERT(val.contains("name"));
        uint16_t idx = parseValueNames(val["name"], allowDuplicates);

        ASSERT(val.contains("group"));
        eckit::Value group = val["group"];
        ASSERT(group.isList());

        std::set<std::string> outSet;
        for (size_t i = 0; i < group.size(); ++i) {
            std::vector<std::string> groupVals = parseEnumValue(group[i], true);
            for (const auto& v : groupVals) {
                if (outSet.find(v) == outSet.end()) {
                    outSet.insert(v);
                    groups_.at(idx).second.push_back(v);
                }
            }
        }
        return groups_.at(idx).second;
    }

    // Single value (with aliases) without group
    uint16_t idx   = parseValueNames(val, allowDuplicates);
    std::string nn = groups_.at(idx).first;
    groups_.at(idx).second.push_back(nn);
    return {nn};
}

void TypeEnum::readValuesFile() const {
    if (!valuesFile_.empty()) {
        auto values = MarsLanguage::jsonFile(valuesFile_);
        ASSERT(values.isList());

        for (size_t i = 0; i < values.size(); ++i) {
            parseEnumValue(values[i]);
        }
    }
}

TypeEnum::TypeEnum(const std::string& type, Keyword key, const eckit::Value& val) : Type(type, key, val) {

    LOG_DEBUG_LIB(LibMetkit) << "TypeEnum name=" << name() << " settings=" << val << std::endl;

    eckit::Value values = val["values"];
    if (!values.isList()) {
        valuesFile_ = static_cast<std::string>(values);
    }
    else {
        for (size_t i = 0; i < values.size(); ++i) {
            parseEnumValue(values[i]);
        }
    }
}

TypeEnum::TypeEnum(const std::string& type, Keyword key, MemFile& file) : Type(type, key, file) {

    uint16_t numGroups = file.read16();
    for (uint16_t i = 0; i < numGroups; ++i) {
        std::string k{file.readString()};
        std::vector<std::string> groupMembers;
        uint16_t numGroupMembers = file.read16();
        for (uint16_t j = 0; j < numGroupMembers; ++j) {
            groupMembers.emplace_back(file.readString());
        }
        values_.emplace(k, groups_.size());
        groups_.emplace_back(k, groupMembers);
    }
    uint16_t numAliases = file.read16();
    for (uint16_t i = 0; i < numAliases; ++i) {
        std::string k{file.readString()};
        uint16_t v = file.read16();
        values_.emplace(k, v);
    }
    valuesFile_ = file.readString();
}

void TypeEnum::write(std::ofstream& file) const {

    Type::write(file);
    std::map<std::string, uint16_t> aliases = values_;
    write16(file, groups_.size());
    for (const auto& [k, vals] : groups_) {
        aliases.erase(k);
        writeString(file, k);
        write16(file, vals.size());
        for (const auto& v : vals) {
            writeString(file, v);
        }
    }
    write16(file, aliases.size());
    for (const auto& [k, v] : aliases) {
        writeString(file, k);
        write16(file, v);
    }
    writeString(file, valuesFile_);
}

void TypeEnum::print(std::ostream& out) const {
    out << "TypeEnum[name=" << name() << "]";
}

bool TypeEnum::expand(std::string& value, const MarsRequest& request) const {
    auto it = find(value);
    if (it == values_.end()) {
        return false;
    }
    ASSERT(groups_.size() > it->second);
    value = groups_.at(it->second).first;
    return true;
}

std::map<std::string, uint16_t>::const_iterator TypeEnum::find(const std::string& value) const {
    std::call_once(readValues_, &TypeEnum::readValuesFile, this);

    return values_.find(eckit::StringTools::lower(value));
}

std::optional<std::reference_wrapper<const std::vector<std::string>>> TypeEnum::group(const std::string& value) const {
    ASSERT(flags_[5]);

    auto it = find(value);
    if (it != values_.end()) {
        return groups_.at(it->second).second;
    }
    return std::nullopt;
}

static TypeBuilder<TypeEnum> type("enum");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
