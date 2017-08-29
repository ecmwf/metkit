/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeEnum.h"
#include "metkit/MarsLanguage.h"
#include "eckit/parser/JSONParser.h"


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeEnum::TypeEnum(const std::string &name, const eckit::Value& settings) :
    Type(name, settings),
    multiple_(false) {

    if (settings.contains("multiple")) {
        multiple_ = settings["multiple"];
    }

    eckit::Value values = settings["values"];

    if (!values.isList()) {
        values = MarsLanguage::jsonFile(values);
        ASSERT(values.isList());
    }

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& val = values[i];

        if (val.isList()) {
            ASSERT(val.size() > 0);

            std::string first = val[0];

            for (size_t j = 0; j < val.size(); ++j) {
                std::string v = val[j];

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


}

TypeEnum::~TypeEnum() {
}

void TypeEnum::print(std::ostream &out) const {
    out << "TypeEnum[name=" << name_ << "]";
}

bool TypeEnum::expand(std::string& value) const {


    std::map<std::string, std::string>::iterator c = cache_.find(value);
    if (c != cache_.end()) {
        value = (*c).second;
        return true;
    }

    std::string v = MarsLanguage::bestMatch(value, values_, false, false, mapping_);
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

} // namespace metkit
