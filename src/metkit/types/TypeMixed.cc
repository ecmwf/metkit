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
#include "metkit/types/TypeMixed.h"


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeMixed::TypeMixed(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {
    eckit::Value types = settings["type"];

    eckit::Value cfg = settings;


    for (size_t i = 0; i < types.size(); ++i) {
        cfg["type"] = types[i];

        Type *k = TypesFactory::build(name + "." + std::string(types[i]), cfg);
        k->attach();
        types_.push_back(k);
    }

}

TypeMixed::~TypeMixed() {
    for (std::vector<Type*>::iterator j = types_.begin(); j != types_.end(); ++j) {
        (*j)->detach();
    }
}

void TypeMixed::print(std::ostream &out) const {
    out << "TypeMixed[name=" << name_;
    for (std::vector<Type*>::const_iterator j = types_.begin(); j != types_.end(); ++j) {
        out << "," << *(*j);
    }
    out  << "]";
}


bool TypeMixed::expand(std::string& value) const {
    for (std::vector<Type*>::const_iterator j = types_.begin(); j != types_.end(); ++j) {
        std::string tmp = value;
        if ((*j)->expand(tmp)) {
            value = tmp;
            return true;
        }
    }

    return false;
}


static TypeBuilder<TypeMixed> type("mixed");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
