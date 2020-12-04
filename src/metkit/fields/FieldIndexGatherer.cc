/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/fields/FieldIndexGatherer.h"

namespace metkit {
namespace fields {

//----------------------------------------------------------------------------------------------------------------------

bool FieldIndexGatherer::operator==(const FieldIndexGatherer& rhs) {
    return (stringValues_ == rhs.stringValues_ && doubleValues_ == rhs.doubleValues_ &&
            longValues_ == rhs.longValues_);
}

void FieldIndexGatherer::setValue(const std::string& name, double value) {
    metkit::fields::FieldIndex::setValue(name, value);
}
void FieldIndexGatherer::setValue(const std::string& name, long value) {
    metkit::fields::FieldIndex::setValue(name, value);
}
void FieldIndexGatherer::setValue(const std::string& name, const std::string& value) {
    metkit::fields::FieldIndex::setValue(name, value);
}

void FieldIndexGatherer::print(std::ostream& s) const {
    s << "{string:[";
    std::string separator{""};
    for(auto v: stringValues_) {
        s << separator << v.first << "=" << v.second;
        separator = ",";
    }
    s << "],long:[";
    separator = "";
    for(auto v: longValues_) {
        s << separator << v.first << "=" << v.second;
        separator = ",";
    }
    s << "],double:[";
    separator = "";
    for(auto v: doubleValues_) {
        s << separator << v.first << "=" << v.second;
        separator = ",";
    }
    s << "]}";
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace fields
}  // namespace metkit
