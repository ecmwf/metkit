/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <set>

#include "eckit/exception/Exceptions.h"
#include "eckit/message/Message.h"
#include "eckit/utils/StringTools.h"

#include "metkit/fields/FieldIndex.h"

namespace metkit {
namespace fields {

FieldIndex::FieldIndex() {}

FieldIndex::FieldIndex(eckit::Stream& s) {
    bool b, more;
    std::string key;
    std::string str;
    long ival;
    double dval;

    s >> more;
    while (more) {
        s >> key;

        s >> b;
        if (b) {
            s >> str;
            stringValues_[key] = str;
        }

        s >> b;
        if (b) {
            s >> ival;
            longValues_[key] = ival;
        }

        s >> b;
        if (b) {
            s >> dval;
            doubleValues_[key] = dval;
        }
        s >> more;
    }
}

FieldIndex::FieldIndex(const eckit::message::Message& message) {
    eckit::message::TypedSetter<FieldIndex> gather(*this);
    message.getMetadata(gather);
}

void FieldIndex::encode(eckit::Stream& s) const {

    std::set<std::string> keys;

    for (auto j = stringValues_.begin(); j != stringValues_.end(); ++j) {
        keys.insert((*j).first);
    }

    for (auto j = longValues_.begin(); j != longValues_.end(); ++j) {
        keys.insert((*j).first);
    }

    for (auto j = doubleValues_.begin(); j != doubleValues_.end(); ++j) {
        keys.insert((*j).first);
    }

    for (auto j = keys.begin(); j != keys.end(); ++j) {


        const std::string& key = (*j);

        s << bool(true);
        s << key;

        auto js = stringValues_.find(key);
        s << bool(js != stringValues_.end());
        if (js != stringValues_.end()) {
            s << (*js).second;
        }

        auto ls = longValues_.find(key);
        s << bool(ls != longValues_.end());
        if (ls != longValues_.end()) {
            s << (*ls).second;
        }

        auto ds = doubleValues_.find(key);
        s << bool(ds != doubleValues_.end());
        if (ds != doubleValues_.end()) {
            s << (*ds).second;
        }
    }

    s << bool(false);
}

FieldIndex::~FieldIndex() {}

std::string FieldIndex::substitute(const std::string& pattern) const {
    return eckit::StringTools::substitute(pattern, stringValues_);
}

void FieldIndex::getValue(const std::string& key, double& value) {
    std::map<std::string, double>::iterator j = doubleValues_.find(key);
    if (j == doubleValues_.end())
        throw eckit::UserError(std::string("FieldIndex::getDouble failed for [") + key + "]");
    value = (*j).second;
}

void FieldIndex::getValue(const std::string& key, long& value) {
    std::map<std::string, long>::iterator j = longValues_.find(key);
    if (j == longValues_.end())
        throw eckit::UserError(std::string("FieldIndex::getLong failed for [") + key + "]");
    value = (*j).second;
}

void FieldIndex::getValue(const std::string& key, std::string& value) {
    std::map<std::string, std::string>::iterator j = stringValues_.find(key);
    if (j == stringValues_.end())
        throw eckit::UserError(std::string("FieldIndex::getString failed for [") + key + "]");
    value = (*j).second;
}

void FieldIndex::setValue(const std::string& name, double value) {
    doubleValues_[name] = value;
}

void FieldIndex::setValue(const std::string& name, long value) {
    longValues_[name] = value;
}

void FieldIndex::setValue(const std::string& name, const std::string& value) {
    stringValues_[name] = value;
}


}  // namespace fields
}  // namespace metkit
