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
#include "metkit/types/TypeRange.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeRange::TypeRange(const std::string &name, const eckit::Value& settings) :
    TypeToByList(name, settings) {
}

TypeRange::~TypeRange() {
}

void TypeRange::print(std::ostream &out) const {
    out << "TypeRange[name=" << name_ << "]";
}

std::string TypeRange::tidy(const std::string &value) const  {
    long a = -1;
    long b = -1;

    long *n = &a;

    for (std::string::const_iterator j = value.begin(); j != value.end(); ++j) {
        switch (*j) {
        case '-':
            if (j != value.begin()) {
                if (n == &b) {
                    throw eckit::UserError(name_ + ": invalid integer range '" + value + "'");

                }
                n = &b;
            }
            else {
                throw eckit::UserError(name_ + ": invalid integer range '" + value + "'");
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
            (*n) *= 10;
            (*n) += (*j) - '0';
            break;


        default:

            throw eckit::UserError(name_ + ": invalid integer range '" + value + "'");            break;
        }
    }

    if (a == -1) {// || b == -1) {
        throw eckit::UserError(name_ + ": invalid integer range '" + value + "'");
    }

    if(b == -1) {
         std::ostringstream oss;
    oss << a << "-" << b;

    return oss.str();
    }

    std::ostringstream oss;
    oss << a << "-" << b;

    return oss.str();
}


static TypeBuilder<TypeRange> type("integer");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
