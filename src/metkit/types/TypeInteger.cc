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
#include "metkit/types/TypeInteger.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeInteger::TypeInteger(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {
}

TypeInteger::~TypeInteger() {
}

void TypeInteger::print(std::ostream &out) const {
    out << "TypeInteger[name=" << name_ << "]";
}

bool TypeInteger::ok(const std::string &value, long& n) const {
    n = 0;
    long sign = 1;
    for (std::string::const_iterator j = value.begin(); j != value.end(); ++j) {
        switch (*j) {
        case '-':
            if (j == value.begin()) {
                sign = -1;
            }
            else {
                return false;
            }
            break;

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
            n *= 10;
            n += (*j) - '0';
            break;

        default:
            return false;
            break;
        }
    }
    n *= sign;
    return true;
}

bool TypeInteger::expand( std::string &value) const  {
    long n = 0;
    if (ok(value, n)) {
        static eckit::Translator<long, std::string> l2s;
        value = l2s(n);
        return true;
    }
    return false;
}

static TypeBuilder<TypeInteger> type("integer");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
