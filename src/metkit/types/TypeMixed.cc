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
    TypeEnum(name, settings) {
    eckit::Value types = settings["more"];
    for (size_t i = 0; i < types.size(); ++i) {
        Type *k = TypesFactory::build(name, types[i]);
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
    out << "TypeMixed[name=" << name_ << "]";
}

std::string TypeMixed::tidy(const std::string &value) const {
    NOTIMP;
}

void TypeMixed::expand(const MarsRequest& request, std::vector<std::string>& values) const {
    if (!TypeEnum::expand(values, false)) {
        for (std::vector<Type*>::const_iterator j = types_.begin(); j != types_.end(); ++j) {
            (*j)->expand(request, values);
        }
    }
}


static TypeBuilder<TypeMixed> type("enum-or-more");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
