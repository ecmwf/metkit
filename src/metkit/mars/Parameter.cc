// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <iterator>
#include <ostream>

#include "eckit/exception/Exceptions.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Parameter.h"
#include "metkit/mars/Type.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

Parameter::Parameter(MarsKeyword key, const std::string& name, const std::vector<std::string>& values) :
    key_(key), name_(name), values_(values) {}

Parameter::Parameter(MarsKeyword key, const std::string& name, std::vector<std::string>&& values) :
    key_(key), name_(name), values_(std::move(values)) {}

Parameter::Parameter(const Type& type, const std::vector<std::string>& values) :
    key_(type.id()), name_(type.name()), values_(values), type_(std::cref(type)) {}

Parameter::Parameter(const Type& type, std::vector<std::string>&& values) :
    key_(type.id()), name_(type.name()), values_(std::move(values)), type_(std::cref(type)) {}

bool Parameter::is(MarsKeyword key) const {
    if (key_ != MarsKeyword{}) {
        return key_ == key;
    }
    return key != 0 && name_ == MarsLanguage::name(key);
}

const Type& Parameter::type() const {
    if (!type_) {
        throw eckit::SeriousBug("Parameter '" + name_ + "' is untyped", Here());
    }
    return type_.value();
}

bool Parameter::operator<(const Parameter& other) const {
    if (name() != other.name()) {
        return name() < other.name();
    }
    return values() < other.values();
}

bool Parameter::multiple() const {
    return !type_ || type_.value().get().multiple();
}

size_t Parameter::count() const {
    return type_ ? type_.value().get().count(values_) : values_.size();
}

bool Parameter::filter(const std::vector<std::string>& filter) {
    if (type_) {
        return type_.value().get().filter(filter, values_);
    }

    NotInSet not_in_set(filter);
    values_.erase(std::remove_if(values_.begin(), values_.end(), not_in_set), values_.end());
    return !values_.empty();
}

bool Parameter::filter(MarsKeyword keyword, const std::vector<std::string>& f) {
    if (type_) {
        return type_.value().get().filter(keyword, f, values_);
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
        return type_.value().get().matches(match, values_);
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
