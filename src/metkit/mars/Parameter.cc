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
#include <ostream>

#include "eckit/exception/Exceptions.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Parameter.h"
#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

Parameter::Parameter(const std::string& name, const std::vector<std::string>& values) : name_(name), values_(values) {}

Parameter::Parameter(const std::string& name, std::vector<std::string>&& values) :
    name_(name), values_(std::move(values)) {}

Parameter::Parameter(std::shared_ptr<const Type> type, const std::vector<std::string>& values) :
    values_(values), type_(std::move(type)) {
    ASSERT(type_);
}

Parameter::Parameter(std::shared_ptr<const Type> type, std::vector<std::string>&& values) :
    values_(std::move(values)), type_(std::move(type)) {
    ASSERT(type_);
}

Keyword Parameter::id() const {
    // An untyped parameter may carry an arbitrary custom key that is not in the language definition: do not register
    // it, the dictionary is global, never shrinks and its size is limited, so user-provided names must not fill it.
    // A name that is not registered has no id (0), which no keyword has.
    return type_ ? type_->id() : MarsLanguage::hasKeyword(name_);
}

const std::string& Parameter::name() const {
    return type_ ? type_->name() : name_;
}

bool Parameter::is(Keyword key) const {
    if (type_) {
        return type_->id() == key;
    }
    return key != 0 && name_ == MarsLanguage::name(key);
}

const Type& Parameter::type() const {
    if (!type_) {
        throw eckit::SeriousBug("Parameter '" + name_ + "' is untyped", Here());
    }
    return *type_;
}

bool Parameter::operator<(const Parameter& other) const {
    if (name() != other.name()) {
        return name() < other.name();
    }
    return values() < other.values();
}

bool Parameter::multiple() const {
    return !type_ || type_->multiple();
}

size_t Parameter::count() const {
    return type_ ? type_->count(values_) : values_.size();
}

bool Parameter::filter(const std::vector<std::string>& filter) {
    if (type_) {
        return type_->filter(filter, values_);
    }

    NotInSet not_in_set(filter);
    values_.erase(std::remove_if(values_.begin(), values_.end(), not_in_set), values_.end());
    return !values_.empty();
}

bool Parameter::filter(Keyword keyword, const std::vector<std::string>& f) {
    if (type_) {
        return type_->filter(keyword, f, values_);
    }

    // an untyped parameter has no knowledge of filters by another keyword (e.g. filtering a date by day): it can only
    // be filtered by its own keyword, anything else is a "no match" and not an error
    return keyword == id() && filter(f);
}

bool Parameter::filter(const std::string& name, const std::vector<std::string>& f) {
    return filter(MarsLanguage::keyword(name), f);
}

bool Parameter::matches(const std::vector<std::string>& match) const {
    if (type_) {
        return type_->matches(match, values_);
    }

    // same semantics as Type::matches(): at least one of the values is among the ones to match
    return std::any_of(values_.begin(), values_.end(), [&match](const std::string& v) {
        return std::find(match.begin(), match.end(), v) != match.end();
    });
}

void Parameter::merge(const Parameter& p) {
    ASSERT(name() == p.name());

    /// @note this isn't optimal O(N^2) but it respects the order

    std::vector<std::string> diff;
    for (const auto& o : p.values()) {
        if (std::find(values_.begin(), values_.end(), o) == values_.end()) {
            diff.push_back(o);
        }
    }

    values_.insert(values_.end(), std::make_move_iterator(diff.begin()), std::make_move_iterator(diff.end()));
}

void Parameter::print(std::ostream& s) const {
    if (type_) {
        s << "Parameter[type=" << *type_;
    }
    else {
        s << "Parameter[name=" << name_;
    }
    s << ",values=[";
    const char* separator = "";
    for (const auto& v : values_) {
        s << separator << v;
        separator = ",";
    }
    s << "]]";
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
