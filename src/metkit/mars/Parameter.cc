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

#include "metkit/mars/MarsLanguage.h"
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

ParameterBase::ParameterBase(const Parameter& other) {
    values_ = other.values();
}

void ParameterBase::values(const std::vector<std::string>& values) {
    values_ = values;
}

size_t ParameterBase::count() const {
    return values_.size();
}

bool ParameterBase::multiple() const {
    return true;
}


bool ParameterBase::filter(const std::vector<std::string>& filter) {
    NotInSet not_in_set(filter);

    values_.erase(std::remove_if(values_.begin(), values_.end(), not_in_set), values_.end());

    return !values_.empty();
}

bool ParameterBase::filter(Keyword keyword, const std::vector<std::string>& f) {
    if (keyword != id()) {
        // raw (untyped) parameters have no Type-specific knowledge of keyword-based filters
        // (e.g. filtering "date" by "day"), so - like the old TypeAny/undefined-type path - treat
        // an unsupported filter keyword as "no match" rather than as an error.
        return false;
    }
    return filter(f);
}

bool ParameterBase::matches(const std::vector<std::string>& match) const {
    // same semantics as Type::matches(): at least one of the values is among the ones to match
    return std::any_of(values_.begin(), values_.end(), [&match](const std::string& v) {
        return std::find(match.begin(), match.end(), v) != match.end();
    });
}


void ParameterBase::merge(const Parameter& p) {
    ASSERT(name() == p.name());

    /// @note this isn't optimal O(N^2) but it respects the order

    std::vector<std::string> diff;
    for (auto& o : p.values()) {
        bool found = false;
        for (auto& v : values()) {
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

Parameter::Parameter(const std::string& name, const std::vector<std::string>& values) {
    impl_ = std::make_unique<StringParameter>(name, values);
}


Parameter::Parameter(const Parameter& other) : impl_(other.impl_ ? other.impl_->clone() : nullptr) {}

Parameter& Parameter::operator=(const Parameter& other) {
    if (this != &other) {
        impl_ = other.impl_ ? other.impl_->clone() : nullptr;
    }
    return *this;
}

Parameter::Parameter(std::unique_ptr<ParameterBase>&& param) {
    impl_ = std::move(param);
}

bool Parameter::operator<(const Parameter& other) const {
    if (name() != other.name()) {
        return name() < other.name();
    }
    return values() < other.values();
}

bool Parameter::filter(const std::string& name, const std::vector<std::string>& filter) {
    return impl_->filter(MarsLanguage::keyword(name), filter);
}

// Parameter& Parameter::operator=(Parameter&& other) {
//     impl_ = std::move(other.impl_);
//     return *this;
// }

// Parameter& Parameter::operator=(std::unique_ptr<ParameterBase>&& other) {
//     impl_ = std::move(other);
//     return *this;
// }

//----------------------------------------------------------------------------------------------------------------------


StringParameter& StringParameter::operator=(const StringParameter& other) {
    name_   = other.name_;
    values_ = other.values_;
    return *this;
}

Keyword StringParameter::id() const {
    // raw (unvalidated) parameters may carry arbitrary custom keys that are not in the language definition. Do not
    // intern them: the dictionary is global, never shrinks and its keys are limited, so user-provided names must not
    // be able to fill it. A keyword that is not registered has no id (0), which no validated request can contain.
    return MarsLanguage::hasKeyword(name_);
}

void StringParameter::print(std::ostream& s) const {
    s << "StringParameter[name=" << name_ << ",values=" << values_ << "]";
}

//----------------------------------------------------------------------------------------------------------------------


// TypeParameter::TypeParameter() : type_(&undefined) {
//     type_->attach();
// }

TypeParameter::~TypeParameter() = default;

TypeParameter::TypeParameter(const std::vector<std::string>& values, std::shared_ptr<const Type> type) :
    ParameterBase(values), type_(type) {}
//     // if (!type) {
//     //     type_ = &undefined;
//     // }
//     type_->attach();
// }


TypeParameter::TypeParameter(const TypeParameter& other) : ParameterBase(other.values_), type_(other.type_) {}

TypeParameter& TypeParameter::operator=(const TypeParameter& other) {
    type_   = other.type_;
    values_ = other.values_;
    return *this;
}

Keyword TypeParameter::id() const {
    return type_->id();
}

const std::string& TypeParameter::name() const {
    return type_->name();
}

bool TypeParameter::multiple() const {
    return type_->multiple();
}

bool TypeParameter::filter(const std::vector<std::string>& filter) {
    return type_->filter(filter, values_);
}

bool TypeParameter::filter(Keyword keyword, const std::vector<std::string>& filter) {
    return type_->filter(keyword, filter, values_);
}

bool TypeParameter::matches(const std::vector<std::string>& match) const {
    return type_->matches(match, values_);
}

size_t TypeParameter::count() const {
    return type_->count(values_);
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
