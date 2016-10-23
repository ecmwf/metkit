/*
 * (C) Copyright 1996-2016 ECMWF.
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


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeEnum::TypeEnum(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {

    eckit::Value values = settings["values"];
    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& val = values[i];

        if (val.isList()) {
            ASSERT(val.size() > 0);

            std::string first = val[0];

            for (size_t j = 0; j < val.size(); ++j) {
                std::string v = val[j];
                mapping_[v] = first;
                values_.push_back(v);
            }
        }
        else {
            std::string v = val;
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

void TypeEnum::expand(std::vector<std::string>& values) const {

    std::vector<std::string> newval;
    for (std::vector<std::string>::const_iterator j = values.begin(); j != values.end(); ++j) {
        std::string v = MarsLanguage::bestMatch((*j), values_, mapping_);
        std::map<std::string, std::string>::const_iterator k = mapping_.find(v);
        ASSERT(k != mapping_.end());
        newval.push_back((*k).second);
    }
    std::swap(values, newval);
}

static TypeBuilder<TypeEnum> type("enum");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
