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

std::string TypeInteger::tidy(const std::string &value) const  {
    static eckit::Translator<std::string, long> s2l;
    static eckit::Translator<long, std::string> l2s;
    long l = s2l(value);
    if (l == 0) {
        ASSERT(value == "0");
    }
    return l2s(l);
}


static TypeBuilder<TypeInteger> type("integer");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
