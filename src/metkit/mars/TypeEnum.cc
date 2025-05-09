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

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypeEnum.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeEnum::TypeEnum(const std::string& name, const eckit::Value& settings) : Type(name, settings) {

    LOG_DEBUG_LIB(LibMetkit) << "TypeEnum name=" << name << " settings=" << settings << std::endl;

    eckit::Value values = settings["values"];

    if (!values.isList()) {
        values = MarsLanguage::jsonFile(values);
        ASSERT(values.isList());
    }

    for (size_t i = 0; i < values.size(); ++i) {
        const eckit::Value& val = values[i];

        // LOG_DEBUG_LIB(LibMetkit) << "val : " << val << std::endl;

        if (val.isList()) {
            ASSERT(val.size() > 0);

            std::string first = val[0];

            for (size_t j = 0; j < val.size(); ++j) {
                std::string v = val[j];
                //                LOG_DEBUG_LIB(LibMetkit) << "v[" << j << "] : " << v << std::endl;
                if (mapping_.find(v) != mapping_.end()) {
                    std::ostringstream oss;
                    oss << "Redefined enum '" << v << "', '" << first << "' and '" << mapping_[v] << "'";
                    throw eckit::SeriousBug(oss.str());
                }

                mapping_[v] = first;
                values_.push_back(v);
            }
        }
        else {
            std::string v = val;
            if (mapping_.find(v) != mapping_.end()) {
                std::ostringstream oss;
                oss << "Redefined enum '" << v << "' and '" << mapping_[v] << "'";
                throw eckit::SeriousBug(oss.str());
            }
            mapping_[v] = v;
            values_.push_back(v);
        }
    }
    LOG_DEBUG_LIB(LibMetkit) << "TypeEnum name=" << name << " mapping " << mapping_ << std::endl;
}

void TypeEnum::print(std::ostream& out) const {
    out << "TypeEnum[name=" << name_ << "]";
}

bool TypeEnum::expand(const MarsExpandContext& ctx, const MarsRequest& /* request */, std::string& value) const {
    std::map<std::string, std::string>::iterator c = cache_.find(value);
    if (c != cache_.end()) {
        value = (*c).second;
        return true;
    }

    std::string v = MarsLanguage::bestMatch(ctx, value, values_, false, false, false, mapping_);
    if (v.empty()) {
        return false;
    }

    std::map<std::string, std::string>::const_iterator k = mapping_.find(v);
    ASSERT(k != mapping_.end());
    value = cache_[value] = (*k).second;

    return true;
}


void TypeEnum::reset() {
    // cache_.clear();
    Type::reset();
}

static TypeBuilder<TypeEnum> type("enum");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit
