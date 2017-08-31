/*
 * (C) Copyright 1996-2017 ECMWF.
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
    flatten_(true),
    multiple_(false) {

    if (settings.contains("multiple")) {
        multiple_ = settings["multiple"];
    }

    if (settings.contains("flatten")) {
        flatten_ = settings["flatten"];
    }

    if (settings.contains("category")) {
        category_ = std::string(settings["category"]);
    }

    if (settings.contains("default")) {
        eckit::Value d = settings["default"];
        if (!d.isNil()) {
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
    }

    originalDefaults_ = defaults_;

    if (settings.contains("only")) {
        eckit::Value d = settings["only"];

        size_t len = d.size();
        for (size_t i = 0; i < len; i++) {
            eckit::Value a = d[i];
            eckit::Value keys = a.keys();

            for (size_t j = 0; j < keys.size(); j++) {
                std::string key = keys[i];
                eckit::Value v = a[key];

                if (v.isList())
                {
                    for (size_t k = 0; k < v.size(); k++) {
                        only_[key].insert(v[k]);
                    }
                }
                else {
                    only_[key].insert(v);
                }
            }
        }
    }

    if (settings.contains("never")) {
        eckit::Value d = settings["never"];

        size_t len = d.size();
        for (size_t i = 0; i < len; i++) {
            eckit::Value a = d[i];
            eckit::Value keys = a.keys();

            for (size_t j = 0; j < keys.size(); j++) {
                std::string key = keys[i];
                eckit::Value v = a[key];

                if (v.isList())
                {
                    for (size_t k = 0; k < v.size(); k++) {
                        never_[key].insert(v[k]);
                    }
                }
                else {
                    never_[key].insert(v);
                }
            }
        }
    }

}

Type::~Type() {
}

bool Type::flatten() const {
    return flatten_;
}

size_t Type::count(const std::vector<std::string>& values) const {
    return flatten_ ? values.size() : 1;
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

bool Type::filter(const std::vector<std::string> &filter, std::vector<std::string> &values) const {

    NotInSet not_in_set(filter);

    values.erase(std::remove_if(values.begin(), values.end(), not_in_set), values.end());

    return !values.empty();

}

class InSet {

    std::set<std::string> set_;
public:

    InSet(const std::vector<std::string>& f):
        set_(f.begin(), f.end()) {}

    bool operator()(const std::string& s) const {
        return set_.find(s) != set_.end();
    }
};

bool Type::matches(const std::vector<std::string> &match, const std::vector<std::string> &values) const {
    InSet in_set(match);
    return std::find_if(values.begin(), values.end(), in_set) !=  values.end();
}


std::ostream &operator<<(std::ostream &s, const Type &x) {
    x.print(s);
    return s;
}


std::string Type::tidy(const std::string &value) const {
    std::string result = value;
    if (!expand(result)) {

    }
    return result;
}


bool Type::expand(std::string& value) const {
    std::ostringstream oss;
    oss << *this << ":  expand not implemented (" << value << ")";
    throw eckit::SeriousBug(oss.str());
}

void Type::expand(std::vector<std::string>& values) const {
    std::vector<std::string> newvals;

    for (std::vector<std::string>::const_iterator j = values.begin(); j != values.end(); ++j) {

        std::string value = *j;
        if (!expand(value)) {
            std::ostringstream oss;
            oss << *this << ": cannot expand '" << *j << "'";
            throw eckit::UserError(oss.str());
        }

        newvals.push_back(value);
    }

    std::swap(newvals, values);

    if (!multiple_ && values.size() > 1) {
        throw eckit::UserError("Only one value passible for '" + name_ + "'");
    }
}

void Type::setDefaults(MarsRequest& request) {
    if (!defaults_.empty()) {
        request.setValuesTyped(this, defaults_);
    }
}

void Type::setDefaults(const std::vector<std::string>& defaults) {
    defaults_ = defaults;
}

const std::vector<std::string>& Type::flattenValues(const MarsRequest& request) {
    return request.values(name_);
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


const std::string& Type::category() const {
    return category_;
}

void Type::pass2(MarsRequest& request) {
}

void Type::finalise(MarsRequest& request) {
    bool ok = true;

    const std::vector<std::string>& values = request.values(name_, true);
    if (values.size() == 1 && values[0] == "off") {
        ok = false;
    }

    for (std::map<std::string, std::set<std::string> >::const_iterator
            j = only_.begin(); ok && j != only_.end(); ++j) {

        const std::string& name = (*j).first;
        const std::set<std::string>& only = (*j).second;

        const std::vector<std::string>& values = request.values(name, true);
        for (std::vector<std::string>::const_iterator k = values.begin(); ok && k != values.end(); ++k) {
            if (only.find(*k) == only.end()) {
                ok = false;
            }
        }
    }

    for (std::map<std::string, std::set<std::string> >::const_iterator
            j = never_.begin(); ok && j != never_.end(); ++j) {

        const std::string& name = (*j).first;
        const std::set<std::string>& never = (*j).second;

        const std::vector<std::string>& values = request.values(name, true);
        for (std::vector<std::string>::const_iterator k = values.begin(); ok && k != values.end(); ++k) {
            if (never.find(*k) != never.end()) {
                ok = false;
            }
        }
    }

    if (!ok) {
        request.unsetValues(name_);
    }

}

void Type::check(const std::vector<std::string>& values) const {
    if (flatten_) {
        std::set<std::string> s(values.begin(), values.end());
        if (values.size() != s.size()) {
            std::cerr << "Duplicate values in " << values << std::endl;
        }
    }
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
