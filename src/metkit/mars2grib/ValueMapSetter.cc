/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars2grib/ValueMapSetter.h"


namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

ValueMapSetter::ValueMapSetter(eckit::ValueMap& map) :
    map_{map} {}


void ValueMapSetter::setValue(const std::string& key, const std::string& value) {
    map_.insert_or_assign(key, value);
}
void ValueMapSetter::setValue(const std::string& key, long value) {
    map_.insert_or_assign(key, value);
}
void ValueMapSetter::setValue(const std::string& key, double value) {
    map_.insert_or_assign(key, value);
}
void ValueMapSetter::setValue(const std::string& key, NullOrMissing) {
    map_.insert_or_assign(key, eckit::Value{});
}

void ValueMapSetter::print(std::ostream& os) const {
    os << "ValueMapSetter{";
    os << map_;
    os << "ValueMapSetter}";
}

//----------------------------------------------------------------------------------------------------------------------

OrderedValueMapSetter::OrderedValueMapSetter(eckit::ValueMap& map, std::deque<std::string>& keys) :
    map_{map}, keys_{keys} {}


void OrderedValueMapSetter::setValue(const std::string& key, const std::string& value) {
    map_.insert_or_assign(key, value);
    keys_.push_back(key);
}
void OrderedValueMapSetter::setValue(const std::string& key, long value) {
    map_.insert_or_assign(key, value);
    keys_.push_back(key);
}
void OrderedValueMapSetter::setValue(const std::string& key, double value) {
    map_.insert_or_assign(key, value);
    keys_.push_back(key);
}
void OrderedValueMapSetter::setValue(const std::string& key, NullOrMissing) {
    map_.insert_or_assign(key, eckit::Value{});
    keys_.push_back(key);
}

void OrderedValueMapSetter::print(std::ostream& os) const {
    os << "OrderedValueMapSetter{";
    bool first=true;
    for (const auto& key: keys_) {
        if (!first) {
            os << ", ";
        }
        auto searchKey = map_.find(key);
        ASSERT(searchKey != map_.end());
        os << key << "=" << searchKey->second;
        first = false;
    }
    os << "}";
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib
