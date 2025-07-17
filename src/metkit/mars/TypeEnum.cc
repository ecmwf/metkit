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


namespace metkit {
namespace mars {

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
    int16_t idx = groups_.size();
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

std::vector<std::string> TypeEnum::parseEnumValue(const std::string& name, const eckit::Value& val, bool uppercase, bool allowDuplicates) {

    if (val.isMap()) {
        hasGroups_ = true;

        // std::vector<std::string> out;
        std::set<std::string> outSet;

        ASSERT(val.contains("name"));
        int16_t idx = parseValueNames(val["name"], uppercase, allowDuplicates);

        ASSERT(val.contains("group"));
        eckit::Value group = val["group"];
        ASSERT(group.isList());

        // auto gg = groups_.at(idx);
        for (size_t i = 0; i < group.size(); ++i) {
            const eckit::Value& v              = group[i];
            std::vector<std::string> groupVals = parseEnumValue(groups_.at(idx).first, v, uppercase, true);
            // std::cout << "Received: " << groupVals << std::endl;
            for (const auto& v : groupVals) {
                if (outSet.find(v) == outSet.end()) {
                    outSet.insert(v);
                    groups_.at(idx).second.push_back(v);
                }
            }
        }

        // TODO
        // mapping_[firstName] = out;
        // values.insert(firstName);
        // gg.second = out;
        // if (name_ == "obstype")
        //     std::cout << "group " << idx << "  " << groups_.at(idx).first <<  "  " << groups_.at(idx).second << std::endl;
        return groups_.at(idx).second;
    }
    else {
        int16_t idx = parseValueNames(val, uppercase, allowDuplicates);
        std::string nn = groups_.at(idx).first;
        groups_.at(idx).second.push_back(nn);
        // if (name_ == "obstype")
        //     std::cout << "leaf " << idx << "  " << groups_.at(idx).first <<  "  " << groups_.at(idx).second << "  Returning: " << nn << std::endl;
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
    // std::map<std::string, std::set<std::string>> map2set;
    for (size_t i = 0; i < values.size(); ++i) {
        parseEnumValue(name, values[i], uppercase);
    }

    // if (name_ == "obstype") {
    //     std::cout << values_ << std::endl;
    //     std::cout << groups_ << std::endl;
    // }
    // for (const auto& v : valuesSet) {
    //     values_.push_back(v);
    // }
}

void TypeEnum::print(std::ostream& out) const {
    out << "TypeEnum[name=" << name_ << "]";
}

bool TypeEnum::expand(const MarsExpandContext& ctx, std::string& value, const MarsRequest& request) const {
    int16_t idx = find(value);
    if (idx < 0) {
        return false;
        // std::ostringstream oss;
        // oss << "Value '" << value << "' not found in type '" << name_;
        // throw eckit::UserError(oss.str(), Here());
    }
    ASSERT(groups_.size() > idx);
    value = groups_.at(idx).first;
    return true;
}

int16_t TypeEnum::find(std::string& value) const {
    std::string val = eckit::StringTools::lower(value);
    auto it = values_.find(val);
    if (it == values_.end()) {
        return -1;
    }
    return it->second;
}

// std::vector<std::string> TypeEnum::expand(const MarsExpandContext&, const std::string& value,
//                                           const MarsRequest&) const {
//     std::string val = eckit::StringTools::lower(value);
//     auto c          = cache_.find(val);
//     if (c != cache_.end()) {
//         return {(*c).second};
//     }

//     auto it = values_.find(val);
//     if (it == values_.end()) {
//         std::ostringstream oss;
//         oss << "Value '" << value << "' not found in type '" << name_;
//         throw eckit::UserError(oss.str(), Here());
//     }
//     auto v = MarsLanguage::bestMatch(ctx, val, values_, false, false, true, mapping_);
//     switch (v.size()) {
//         case 0: {
//             return {};
//         }
//         case 1: {
//             if (v[0].empty()) {
//                 return {};
//             }
//             StringManyMap::const_iterator k = mapping_.find(eckit::StringTools::lower(v[0]));
//             ASSERT(k != mapping_.end());
//             cache_[val] = (*k).second;
//             return {(*k).second};
//         }
//         default: {
//             cache_[val] = v;
//             return v;
//         }
//     }
// }

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

}  // namespace mars
}  // namespace metkit
