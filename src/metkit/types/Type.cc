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
    name_(name),
    flatten_(true) {

    // if (settings.contains("multiple")) {
    //     flatten_ = settings["multiple"];
    // }

    if (settings.contains("flatten")) {
        flatten_ = settings["flatten"];
    }

    if (settings.contains("default")) {
        eckit::Value d = settings["default"];
        if (d.isList()) {
            size_t len = d.size();
            for (size_t i = 0; i < len; i++) {
                defaults_.push_back(d[i]);
            }
        }
        else {
            defaults_.push_back(d);
        }
    }

    originalDefaults_ = defaults_;
}

Type::~Type() {
}

bool Type::flatten() const {
    return flatten_;
}

class NotInSet {

    std::set<std::string> set_;
public:

    NotInSet(const std::vector<std::string>& f):
        set_(f.begin(), f.end()) {}

    bool operator()(const std::string& s) const {
        return set_.find(s) == set_.end();
    }
};

bool Type::filter(const std::vector<std::string> &filter, std::vector<std::string> &values) {

   NotInSet not_in_set(filter);

    values.erase(std::remove_if(values.begin(), values.end(), not_in_set), values.end());

    return !values.empty();

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

    // std::cout << "expand " << name_ << " " << values << " " << newvals << std::endl;

    std::swap(newvals, values);

}

void Type::setDefaults(MarsRequest& request) {
    if (!defaults_.empty()) {
        request.setValuesTyped(this, defaults_);
    }
}

void Type::setDefaults(const std::vector<std::string>& defaults) {
    defaults_ = defaults;
}

void Type::flattenValues(const MarsRequest& request, std::vector<std::string>& values) {
    values = request.values(name_);
}


void Type::clearDefaults() {
    defaults_.clear();
}

void Type::reset() {
    defaults_ = originalDefaults_;
}

const std::string& Type::name() const {
    return name_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
