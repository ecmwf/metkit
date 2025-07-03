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

std::vector<std::string> TypeEnum::parseEnumValue(const std::string& name, const eckit::Value& val, bool uppercase) {

    std::vector<std::string> out;

    if (val.isMap()) { // this is a group
        if (!val.contains("vals")) {
            std::ostringstream oss;
            oss << "Missing nested values in the definition of '" << name << "'";
            throw eckit::SeriousBug(oss.str());
        }
        eckit::Value values = val["vals"];
        ASSERT(values.isList());
        std::set<std::string> outSet;
        for (size_t i = 0; i < values.size(); ++i) {
            const eckit::Value& v = values[i];
            auto o = parseEnumValue(name, v, uppercase);
            for (const auto& vv : o) {
                if (outSet.find(vv) == outSet.end()) {
                    outSet.insert(vv);
                    out.push_back(vv);
                }
            }
        }

        // handle aliases
        if (val.contains("alias")) {
            eckit::Value aliases = val["alias"];
            // if (aliases.isList()) {
            //     for (size_t i = 0; i < aliases.size(); ++i) {
            //         const eckit::Value& alias = aliases[i];
            //         if (alias.isString()) {
            //             std::string a = alias;
            //             if (uppercase) {
            //                 a = eckit::StringTools::upper(a);
            //             }
            //             if (outSet.find(a) == outSet.end()) {
            //                 outSet.insert(a);
            //                 out.push_back(a);
            //             }
            //         }
            //     }
            // }
        }
        return out;
    }
    else {
        if (val.isList()) { // this is a list of aliases
            ASSERT(val.size() > 0);

            std::string first = val[0];
            if (uppercase) {
                first = eckit::StringTools::upper(first);
            }

            for (size_t j = 0; j < val.size(); ++j) {
                std::string VV = val[j];
                std::string v  = eckit::StringTools::lower(VV);
                //                LOG_DEBUG_LIB(LibMetkit) << "v[" << j << "] : " << v << std::endl;
                if (mapping_.find(v) != mapping_.end()) {
                    std::ostringstream oss;
                    oss << "Redefined enum '" << VV << "', '" << first << "' and '" << mapping_[v] << "'";
                    throw eckit::SeriousBug(oss.str());
                }

                mapping_[v].push_back(first);
                values_.push_back(v);
            }
        }
        else {
            std::string VV = val;
            if (uppercase) {
                VV = eckit::StringTools::upper(VV);
            }
            std::string v = eckit::StringTools::lower(VV);
            if (mapping_.find(v) != mapping_.end()) {
                std::ostringstream oss;
                oss << "Redefined enum '" << VV << "' and '" << mapping_[v] << "'";
                throw eckit::SeriousBug(oss.str());
            }
            mapping_[v].push_back(VV);
            values_.push_back(v);
        }
    }

    return out;
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
    std::map<std::string, std::set<std::string>> map2set;

    for (size_t i = 0; i < values.size(); ++i) {
        parseEnumValue(name, values[i], uppercase);
    }

    //     const eckit::Value& val = values[i];

    //     // LOG_DEBUG_LIB(LibMetkit) << "val : " << val << std::endl;

    //     if (val.isMap()) { // this is a group
    //         if (!val.contains("vals")) {
    //             std::ostringstream oss;
    //             oss << "Missing nested values in the definition of '" << name << "', '" << first << "' and '" << mapping_[v] << "'";
    //             throw eckit::SeriousBug(oss.str());


    //         }
    //         eckit::Value values = val["vals"]; 

    //     }
    //     else {
    //         if (val.isList()) { // this is a list of aliases


    //         } else { 

    //         }
    //     }

    //     if (val.isList()) {
    //         ASSERT(val.size() > 0);

    //         std::string first = val[0];
    //         if (uppercase) {
    //             first = eckit::StringTools::upper(first);
    //         }

    //         for (size_t j = 0; j < val.size(); ++j) {
    //             std::string VV = val[j];
    //             std::string v  = eckit::StringTools::lower(VV);
    //             //                LOG_DEBUG_LIB(LibMetkit) << "v[" << j << "] : " << v << std::endl;
    //             if (mapping_.find(v) != mapping_.end()) {
    //                 std::ostringstream oss;
    //                 oss << "Redefined enum '" << VV << "', '" << first << "' and '" << mapping_[v] << "'";
    //                 throw eckit::SeriousBug(oss.str());
    //             }

    //             mapping_[v] = first;
    //             values_.push_back(v);
    //         }
    //     }
    //     else {
    //         std::string VV = val;
    //         if (uppercase) {
    //             VV = eckit::StringTools::upper(VV);
    //         }
    //         std::string v = eckit::StringTools::lower(VV);
    //         if (mapping_.find(v) != mapping_.end()) {
    //             std::ostringstream oss;
    //             oss << "Redefined enum '" << VV << "' and '" << mapping_[v] << "'";
    //             throw eckit::SeriousBug(oss.str());
    //         }
    //         mapping_[v] = VV;
    //         values_.push_back(v);
    //     }
    // }
    LOG_DEBUG_LIB(LibMetkit) << "TypeEnum name=" << name << " mapping " << mapping_ << std::endl;
}

void TypeEnum::print(std::ostream& out) const {
    out << "TypeEnum[name=" << name_ << "]";
}

std::vector<std::string> TypeEnum::expand(const MarsExpandContext& ctx, const std::string& value, const MarsRequest&) const {
    std::string val = eckit::StringTools::lower(value);
    auto c = cache_.find(val);
    if (c != cache_.end()) {
        return {(*c).second};
    }

    auto v = MarsLanguage::bestMatch(ctx, val, values_, false, false, true, mapping_);
    if (v.empty()) {
        return {};
    }

    if (v.size() > 1) {
        cache_[val] = v;
        return v;
    }
    std::map<std::string, std::vector<std::string>>::const_iterator k = mapping_.find(eckit::StringTools::lower(v[0]));
    ASSERT(k != mapping_.end());
    cache_[val] = (*k).second;
    return {(*k).second};
}


void TypeEnum::reset() {
    // cache_.clear();
    Type::reset();
}

static TypeBuilder<TypeEnum> type("enum");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit
