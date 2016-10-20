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

Type::Type(const std::string &name, const std::string &type, const eckit::Value& value) :
    name_(name),
    type_(type),
    value_(value) {
}

Type::~Type() {
}

const std::string &Type::type() const {
    return type_;
}

std::string Type::tidy(const std::string &keyword,
                       const std::string &value) const  {
    return value;
}

void Type::toKey(std::ostream &out,
                 const std::string &keyword,
                 const std::string &value) const {
    out << value;
}

std::ostream &operator<<(std::ostream &s, const Type &x) {
    x.print(s);
    return s;
}

bool Type::match(const std::string&, const std::string& value1, const std::string& value2) const
{
    return (value1 == value2);
}


void Type::expand(std::vector<std::string>& values) const {
    std::vector<std::string> newvals;

    for (std::vector<std::string>::const_iterator j = values.begin(); j != values.end(); ++j) {
        newvals.push_back(tidy(name_, *j));
    }

    std::swap(newvals, values);

}


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
