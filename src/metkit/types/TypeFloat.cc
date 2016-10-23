/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/utils/Translator.h"

#include "metkit/MarsRequest.h"

#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeFloat.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeFloat::TypeFloat(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {
}

TypeFloat::~TypeFloat() {
}

std::string TypeFloat::tidy(const std::string &value) const  {
    static eckit::Translator<std::string, double> s2d;
    static eckit::Translator<double, std::string> d2s;
    return d2s(s2d(value));
}


void TypeFloat::print(std::ostream &out) const {
    out << "TypeFloat[name=" << name_ << "]";
}

static TypeBuilder<TypeFloat> type("float");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
