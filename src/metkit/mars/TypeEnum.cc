/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/parser/JSONParser.h"
#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypeEnum.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

void TypeEnum::addValue(const std::string& value, int16_t idx, bool allowDuplicates) {
    if (!allowDuplicates && values_.find(value) != values_.end()) {
        std::ostringstream oss;
        oss << "Redefined enum value '" << value << "'";
        throw eckit::SeriousBug(oss.str());
    }
    values_[value] = idx;
}

int16_t TypeEnum::parseValueNames(const eckit::Value& names, bool uppercase, bool allowDuplicates) {
    std::string firstName = eckit::StringTools::lower(names.isList() ? names[0] : names);
    int16_t idx           = groups_.size();
    addValue(firstName, idx, allowDuplicates);
    if (names.isList()) {
        for (size_t i = 1; i < names.size(); ++i) {
            addValue(names[i], idx, allowDuplicates);
        }
    }
    if (uppercase) {
        firstName = eckit::StringTools::upper(firstName);
    }
    groups_.emplace_back(firstName, std::vector<std::string>{});
    return idx;
}

std::vector<std::string> TypeEnum::parseEnumValue(const std::string& name, const eckit::Value& val, bool uppercase,
                                                  bool allowDuplicates) {

    if (val.isMap()) {
        hasGroups_ = true;

        ASSERT(val.contains("name"));
        int16_t idx = parseValueNames(val["name"], uppercase, allowDuplicates);

        ASSERT(val.contains("group"));
        eckit::Value group = val["group"];
        ASSERT(group.isList());

        std::set<std::string> outSet;
        for (size_t i = 0; i < group.size(); ++i) {
            const eckit::Value& v              = group[i];
            std::vector<std::string> groupVals = parseEnumValue(groups_.at(idx).first, v, uppercase, true);
            for (const auto& v : groupVals) {
                if (outSet.find(v) == outSet.end()) {
                    outSet.insert(v);
                    groups_.at(idx).second.push_back(v);
                }
            }
        }
        return groups_.at(idx).second;
    }
    else {
        int16_t idx    = parseValueNames(val, uppercase, allowDuplicates);
        std::string nn = groups_.at(idx).first;
        groups_.at(idx).second.push_back(nn);
        return {nn};
    }
}

TypeEnum::TypeEnum(const std::string& name, const eckit::Value& settings) : Type(name, settings), hasGroups_(false) {

    LOG_DEBUG_LIB(LibMetkit) << "TypeEnum name=" << name << " settings=" << settings << std::endl;

    eckit::Value values = settings["values"];
    bool uppercase      = false;
    if (settings.contains("uppercase")) {
        uppercase = settings["uppercase"];
    }

    if (!values.isList()) {
        values = MarsLanguage::jsonFile(values);
        ASSERT(values.isList());
    }
    for (size_t i = 0; i < values.size(); ++i) {
        parseEnumValue(name, values[i], uppercase);
    }
}

void TypeEnum::print(std::ostream& out) const {
    out << "TypeEnum[name=" << name_ << "]";
}

bool TypeEnum::expand(const MarsExpandContext& ctx, std::string& value, const MarsRequest& request) const {
    int16_t idx = find(value);
    if (idx < 0) {
        return false;
    }
    ASSERT(groups_.size() > idx);
    value = groups_.at(idx).first;
    return true;
}

int16_t TypeEnum::find(std::string& value) const {
    std::string val = eckit::StringTools::lower(value);
    auto it         = values_.find(val);
    if (it == values_.end()) {
        return -1;
    }
    return it->second;
}

const std::vector<std::string>& TypeEnum::group(const std::string& value) const {
    ASSERT(hasGroups_);

    auto it = values_.find(value);
    if (it != values_.end()) {
        return groups_.at(it->second).second;
    }
}


void TypeEnum::reset() {
    // cache_.clear();
    Type::reset();
}

static TypeBuilder<TypeEnum> type("enum");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
