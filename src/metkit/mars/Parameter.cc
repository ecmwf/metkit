/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <iterator>

#include "metkit/mars/Parameter.h"
#include "metkit/mars/Type.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class UndefinedType : public Type {
    void print(std::ostream& out) const override { out << "<undefined type>"; }

    bool filter(const std::vector<std::string>&, std::vector<std::string>&) const override { NOTIMP; }
    bool expand(const MarsExpandContext&, const MarsRequest& request, std::string&) const override { NOTIMP; }

public:

    UndefinedType() : Type("<undefined>", eckit::Value()) { attach(); }
};


static UndefinedType undefined;


//----------------------------------------------------------------------------------------------------------------------


Parameter::Parameter() : type_(&undefined) {
    type_->attach();
}

Parameter::~Parameter() {
    type_->detach();
}

Parameter::Parameter(const std::vector<std::string>& values, Type* type) : type_(type), values_(values) {
    if (!type) {
        type_ = &undefined;
    }
    type_->attach();
}


Parameter::Parameter(const Parameter& other) : type_(other.type_), values_(other.values_) {
    type_->attach();
}

Parameter& Parameter::operator=(const Parameter& other) {
    Type* old = type_;
    type_     = other.type_;
    type_->attach();
    old->detach();

    values_ = other.values_;
    return *this;
}

void Parameter::values(const std::vector<std::string>& values) {
    values_ = values;
}

bool Parameter::filter(const std::vector<std::string>& filter) {
    return type_->filter(filter, values_);
}

bool Parameter::filter(const std::string& keyword, const std::vector<std::string>& filter) {
    return type_->filter(keyword, filter, values_);
}


bool Parameter::matches(const std::vector<std::string>& match) const {
    return type_->matches(match, values_);
}

void Parameter::merge(const Parameter& p) {
    ASSERT(name() == p.name());

    /// @note this isn't optimal O(N^2) but it respects the order

    std::vector<std::string> diff;
    for (auto& o : p.values_) {
        bool found = false;
        for (auto& v : values_) {
            if (v == o) {
                found = true;
                break;
            }
        }
        if (!found)
            diff.push_back(o);
    }

    values_.insert(values_.end(), std::make_move_iterator(diff.begin()), std::make_move_iterator(diff.end()));
}


const std::string& Parameter::name() const {
    return type_->name();
}

size_t Parameter::count() const {
    return type_->count(values_);
}

void Parameter::print(std::ostream& s) const {
    s << "Parameter[type=" << *type_ << ",values=" << values_ << "]";
}

bool Parameter::operator<(const Parameter& other) const {
    if (name() != other.name()) {
        return name() < other.name();
    }
    return values_ < other.values_;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit
