/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/types/Type.h"
#include "metkit/MarsRequest.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

Type::Type(const std::string &name, const eckit::Value& settings) :
    name_(name) {
}

Type::~Type() {
}


std::string Type::tidy(const std::string &value) const  {
    return value;
}

std::ostream &operator<<(std::ostream &s, const Type &x) {
    x.print(s);
    return s;
}

void Type::expand(std::vector<std::string>& values) const {
    std::vector<std::string> newvals;

    for (std::vector<std::string>::const_iterator j = values.begin(); j != values.end(); ++j) {
        newvals.push_back(tidy(*j));
    }

    std::swap(newvals, values);

}

void Type::setDefaults(MarsRequest& request) const {
    if (!defaults_.empty()) {
        request.setValues(name_, defaults_);
    }
}

void Type::setDefaults(const std::vector<std::string>& defaults) {
    defaults_ = defaults;
}

void Type::flattenValues(const MarsRequest& request, std::vector<std::string>& values) {
    request.getValues(name_, values);
}


void Type::clearDefaults() {
    defaults_.clear();
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
