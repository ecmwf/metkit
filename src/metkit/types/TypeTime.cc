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

#include "eckit/types/Date.h"
#include "metkit/MarsRequest.h"

#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeTime.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeTime::TypeTime(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {
}

TypeTime::~TypeTime() {
}

std::string TypeTime::tidy(const std::string &value) const {
    eckit::Translator<std::string, long> t;

    long n = t(value);
    if (n < 100) {
        n *= 100;
    }

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << n;
    return oss.str();
}

void TypeTime::print(std::ostream &out) const {
    out << "TypeTime[name=" << name_ << "]";
}

static TypeBuilder<TypeTime> type("time");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
