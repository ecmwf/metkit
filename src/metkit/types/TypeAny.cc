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
#include "metkit/types/TypeAny.h"


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeAny::TypeAny(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {
}

TypeAny::~TypeAny() {
}

void TypeAny::print(std::ostream &out) const {
    out << "TypeAny[name=" << name_ << "]";
}

bool TypeAny::expand( std::string &value) const {
    return true;
}


static TypeBuilder<TypeAny> type("any");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
