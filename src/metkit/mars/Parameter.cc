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

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

// class UndefinedType : public Type {
//     void print(std::ostream& out) const override { out << "<undefined type>"; }

//     bool expand(std::string&, const MarsRequest&) const override { NOTIMP; }

// public:

//     UndefinedType() : Type("<undefined>", eckit::Value()) { attach(); }
// };


// static UndefinedType undefined;


//----------------------------------------------------------------------------------------------------------------------


// TypeParameter::TypeParameter() : type_(&undefined) {
//     type_->attach();
// }

TypeParameter::~TypeParameter() {
    type_->detach();
}

TypeParameter::TypeParameter(const std::vector<std::string>& values, const Type* type) : Parameter(values), type_(type) {
    // if (!type) {
    //     type_ = &undefined;
    // }
    type_->attach();
}


TypeParameter::TypeParameter(const TypeParameter& other) : Parameter(other.values_), type_(other.type_) {
    type_->attach();
}

TypeParameter& TypeParameter::operator=(const TypeParameter& other) {
    const Type* old = type_;
    type_           = other.type_;
    type_->attach();
    old->detach();

    values_ = other.values_;
    return *this;
}

void Parameter::values(const std::vector<std::string>& values) {
    values_ = values;
}

bool Parameter::filter(const std::vector<std::string>& filter) {
    NOTIMP;
}

bool Parameter::filter(Keyword keyword, const std::vector<std::string>& filter) {
    NOTIMP;
}
bool Parameter::matches(const std::vector<std::string>& match) const {
    NOTIMP;
}

bool TypeParameter::filter(const std::vector<std::string>& filter) {
    return type_->filter(filter, values_);
}

bool TypeParameter::filter(Keyword keyword, const std::vector<std::string>& filter) {
    return type_->filter(keyword, filter, values_);
}

size_t Parameter::count() const {
    return values_.size();
}

bool TypeParameter::matches(const std::vector<std::string>& match) const {
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

Keyword TypeParameter::id() const {
    return type_->id();
}

const std::string& TypeParameter::name() const {
    return type_->name();
}

size_t TypeParameter::count() const {
    return type_->count(values_);
}

void StringParameter::print(std::ostream& s) const {
    s << "StringParameter[name=" << name_ << ",values=" << values_ << "]";
}

void TypeParameter::print(std::ostream& s) const {
    s << "TypeParameter[type=" << *type_ << ",values=" << values_ << "]";
}

bool TypeParameter::operator<(const TypeParameter& other) const {
    if (id() != other.id()) {
        return id() < other.id();
    }
    return values_ < other.values_;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
