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

std::vector<std::string> TypeEnum::parseEnumValue(const std::string& name, const eckit::Value& val,
                                                  std::set<std::string>& values, bool uppercase, bool allowDuplicates) {

    if (val.isMap()) {
        deduplicate_ = true;

        std::vector<std::string> out;
        std::set<std::string> outSet;

        ASSERT(val.contains("name"));
        eckit::Value names = val["name"];
        std::string firstName;
        if (names.isList()) {
            firstName = static_cast<std::string>(names[0]);
            if (uppercase) {
                firstName = eckit::StringTools::upper(firstName);
            }
            for (size_t i = 1; i < names.size(); ++i) {
                // handle aliases
            }
        }
        else {
            firstName = static_cast<std::string>(names);
            if (uppercase) {
                firstName = eckit::StringTools::upper(firstName);
            }
        }

        ASSERT(val.contains("group"));
        eckit::Value group = val["group"];
        ASSERT(group.isList());
        for (size_t i = 0; i < group.size(); ++i) {
            const eckit::Value& v              = group[i];
            std::vector<std::string> groupVals = parseEnumValue(firstName, v, values, uppercase, true);
            for (const auto& v : groupVals) {
                if (outSet.find(v) == outSet.end()) {
                    outSet.insert(v);
                    out.push_back(v);
                }
            }
        }

        mapping_[firstName] = out;
        values.insert(firstName);
        return out;
    }

    if (val.isList()) {  // this is a list of aliases
        ASSERT(val.size() > 0);

        std::string first = val[0];
        if (uppercase) {
            first = eckit::StringTools::upper(first);
        }

        for (size_t j = 0; j < val.size(); ++j) {
            std::string VV = val[j];
            std::string v  = eckit::StringTools::lower(VV);
            //                LOG_DEBUG_LIB(LibMetkit) << "v[" << j << "] : " << v << std::endl;
            if (!allowDuplicates && mapping_.find(v) != mapping_.end()) {
                std::ostringstream oss;
                oss << "Redefined enum '" << VV << "', '" << first << "' and '" << mapping_[v] << "'";
                throw eckit::SeriousBug(oss.str());
            }
            if (first != v || mapping_.find(v) == mapping_.end()) {
                mapping_[v].push_back(first);
            }
            values.insert(v);
        }
        return {first};
    }
    else {
        std::string VV = val;
        if (uppercase) {
            VV = eckit::StringTools::upper(VV);
        }
        std::string v = eckit::StringTools::lower(VV);
        if (!allowDuplicates && mapping_.find(v) != mapping_.end()) {
            std::ostringstream oss;
            oss << "Redefined enum '" << VV << "' and '" << mapping_[v] << "'";
            throw eckit::SeriousBug(oss.str());
        }
        if (VV != v || mapping_.find(v) == mapping_.end()) {
            mapping_[v].push_back(VV);
        }
        values.insert(v);

        return {VV};
    }
}

TypeEnum::TypeEnum(const std::string& name, const eckit::Value& settings) : Type(name, settings) {

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

    std::set<std::string> valuesSet;
    for (size_t i = 0; i < values.size(); ++i) {
        parseEnumValue(name, values[i], valuesSet, uppercase);
    }

    values_.reserve(valuesSet.size());
    for (const auto& v : valuesSet) {
        values_.push_back(v);
    }
}

void TypeEnum::print(std::ostream& out) const {
    out << "TypeEnum[name=" << name_ << "]";
}

std::vector<std::string> TypeEnum::expand(const MarsExpandContext& ctx, const std::string& value,
                                          const MarsRequest&) const {
    std::string val = eckit::StringTools::lower(value);
    auto c          = cache_.find(val);
    if (c != cache_.end()) {
        return {(*c).second};
    }

    auto v = MarsLanguage::bestMatch(ctx, val, values_, false, false, true, mapping_);
    switch (v.size()) {
        case 0: {
            return {};
        }
        case 1: {
            if (v[0].empty()) {
                return {};
            }
            StringManyMap::const_iterator k = mapping_.find(eckit::StringTools::lower(v[0]));
            ASSERT(k != mapping_.end());
            cache_[val] = (*k).second;
            return {(*k).second};
        }
        default: {
            cache_[val] = v;
            return v;
        }
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
