/*
 * (C) Copyright 1996-2017 ECMWF.
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

bool TypeFloat::expand(std::string &value) const  {

    for (std::string::const_iterator j = value.begin(); j != value.end(); ++j) {
        switch (*j) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '.':
            break;


        default:
            return false;
            // throw eckit::UserError(name_ + ": invalid float '" + value + "'");
            break;
        }
    }

    static eckit::Translator<std::string, double> s2d;
    static eckit::Translator<double, std::string> d2s;
    value = d2s(s2d(value));
    return true;
}


void TypeFloat::print(std::ostream &out) const {
    out << "TypeFloat[name=" << name_ << "]";
}

static TypeBuilder<TypeFloat> type("float");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
