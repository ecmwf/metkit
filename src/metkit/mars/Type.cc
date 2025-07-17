/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/Type.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/value/Value.h"

#include "metkit/mars/ContextRule.h"
#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypeToByList.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

void Context::add(std::unique_ptr<ContextRule> rule) {
    rules_.push_back(std::move(rule));
}

bool Context::matches(MarsRequest req) const {

    for (const auto& r : rules_) {
        if (!r->matches(req)) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& s, const Context& c) {
    c.print(s);
    return s;
}

void Context::print(std::ostream& out) const {
    std::string sep;
    out << "Context[";
    for (const auto& r : rules_) {
        out << sep << *r;
        sep = ",";
    }
    out << "]";
}

//----------------------------------------------------------------------------------------------------------------------
// HELPERS

std::unique_ptr<ContextRule> parseRule(std::string key, eckit::Value r) {

    std::set<std::string> vals;

    if (r.isList()) {
        for (size_t k = 0; k < r.size(); k++) {
            vals.insert(r[k]);
        }
        return std::make_unique<Include>(key, vals);
    }

    ASSERT(r.contains("op"));
    std::string op = r["op"];
    ASSERT(op.size() == 1);
    switch (op[0]) {
        case 'u':
            return std::make_unique<Undef>(key);
        case 'd':
            return std::make_unique<Def>(key);
        case '!':
            ASSERT(r.contains("values"));
            eckit::Value vv = r["values"];
            for (size_t k = 0; k < vv.size(); k++) {
                vals.insert(vv[k]);
            }
            return std::make_unique<Exclude>(key, vals);
    }
    return nullptr;
}

std::unique_ptr<Context> Context::parseContext(eckit::Value c) {

    std::unique_ptr<Context> context = std::make_unique<Context>();

    eckit::Value keys = c.keys();

    for (size_t j = 0; j < keys.size(); j++) {
        std::string key = keys[j];
        context->add(parseRule(key, c[key]));
    }
    return context;
}

//----------------------------------------------------------------------------------------------------------------------

Type::Type(const std::string& name, const eckit::Value& settings) :
    name_(name), flatten_(true), multiple_(false), duplicates_(true) {

    if (settings.contains("multiple")) {
        multiple_ = settings["multiple"];
    }

    if (settings.contains("flatten")) {
        flatten_ = settings["flatten"];
    }

    if (settings.contains("duplicates")) {
        duplicates_ = settings["duplicates"];
    }

    if (settings.contains("category")) {
        category_ = std::string(settings["category"]);
    }

    if (settings.contains("defaults")) {
        eckit::Value defaults = settings["defaults"];
        if (!defaults.isNil() && defaults.isList()) {

            for (size_t i = 0; i < defaults.size(); i++) {
                std::vector<std::string> vals;
                eckit::Value d = defaults[i];
                ASSERT(d.contains("values"));
                eckit::Value vv = d["values"];

                if (vv.isList()) {
                    for (size_t k = 0; k < vv.size(); k++) {
                        vals.push_back(vv[k]);
                    }
                }
                else {
                    vals.push_back(vv);
                }

                if (d.contains("context")) {
                    defaults_.emplace(Context::parseContext(d["context"]), vals);
                }
                else {
                    defaults_.emplace(std::make_unique<Context>(), vals);
                }
            }
        }
    }
    if (settings.contains("only")) {
        eckit::Value only = settings["only"];
        if (!only.isNil() && only.isList()) {
            for (size_t i = 0; i < only.size(); i++) {
                eckit::Value o = only[i];
                ASSERT(o.contains("context"));
                only_.insert(Context::parseContext(o["context"]));
            }
        }
    }
    if (settings.contains("set")) {
        eckit::Value sets = settings["set"];
        if (!sets.isNil() && sets.isList()) {
            for (size_t i = 0; i < sets.size(); i++) {
                eckit::Value s = sets[i];
                std::vector<std::string> vals;
                ASSERT(s.contains("values"));
                eckit::Value vv = s["values"];
                if (vv.isList()) {
                    for (size_t k = 0; k < vv.size(); k++) {
                        vals.push_back(vv[k]);
                    }
                }
                else {
                    vals.push_back(vv);
                }
                ASSERT(s.contains("context"));
                sets_.emplace(Context::parseContext(s["context"]), vals);
            }
        }
    }
    if (settings.contains("unset")) {
        eckit::Value unsets = settings["unset"];
        if (!unsets.isNil() && unsets.isList()) {
            for (size_t i = 0; i < unsets.size(); i++) {
                eckit::Value u = unsets[i];
                ASSERT(u.contains("context"));
                unsets_.insert(Context::parseContext(u["context"]));
            }
        }
    }
}

bool Type::flatten() const {
    return flatten_;
}

bool Type::multiple() const {
    return multiple_;
}

size_t Type::count(const std::vector<std::string>& values) const {
    return flatten_ ? values.size() : 1;
}

class NotInSet {
    std::set<std::string> set_;

public:

    NotInSet(const std::vector<std::string>& f) : set_(f.begin(), f.end()) {}

    bool operator()(const std::string& s) const { return set_.find(s) == set_.end(); }
};

bool Type::filter(const std::vector<std::string>& filter, std::vector<std::string>& values) const {
    NotInSet not_in_set(filter);

    values.erase(std::remove_if(values.begin(), values.end(), not_in_set), values.end());

    return !values.empty();
}

bool Type::filter(const std::string& keyword, const std::vector<std::string>& f,
                  std::vector<std::string>& values) const {
    if (keyword == name_) {
        return filter(f, values);
    }
    auto it = filters_.find(keyword);
    if (it == filters_.end()) {
        return false;
    }
    return it->second(f, values);
}

class InSet {
    std::set<std::string> set_;

public:

    InSet(const std::vector<std::string>& f) : set_(f.begin(), f.end()) {}

    bool operator()(const std::string& s) const { return set_.find(s) != set_.end(); }
};

bool Type::matches(const std::vector<std::string>& match, const std::vector<std::string>& values) const {
    InSet in_set(match);
    return std::find_if(values.begin(), values.end(), in_set) != values.end();
}


std::ostream& operator<<(std::ostream& s, const Type& x) {
    x.print(s);
    return s;
}

std::string Type::tidy(const std::string& value, const MarsExpandContext& ctx, const MarsRequest& request) const {
    std::string result = value;
    expand(ctx, result, request);
    return result;
}

bool Type::expand(const MarsExpandContext&, std::string& value, const MarsRequest&) const {
    std::ostringstream oss;
    oss << *this << ":  expand not implemented (" << value << ")";
    throw eckit::SeriousBug(oss.str());
}

void Type::expand(const MarsExpandContext& ctx, std::vector<std::string>& values, const MarsRequest& request) const {

    if (toByList_ && values.size() > 1) {
        toByList_->expandRanges(ctx, values, request);
    }

    std::vector<std::string> newvals;
    std::set<std::string> seen;

    for (std::string& val : values) {
        std::string value = val;
        if (!expand(ctx, value, request)) {
            std::ostringstream oss;
            oss << *this << ": cannot expand '" << val << "'" << ctx;
            throw eckit::UserError(oss.str());
        }
        if (hasGroups()) {
            const std::vector<std::string>& gg = group(value);
            for (const auto& v : gg) {
                if (seen.find(v) == seen.end()) {
                    seen.insert(v);
                    newvals.push_back(v);
                }
            }
        }
        else {
            if (!duplicates_ && seen.find(value) != seen.end()) {
                std::ostringstream oss;
                oss << *this << ": duplicated value '" << value << "'" << ctx;
                throw eckit::UserError(oss.str());
            }
            newvals.push_back(value);
        }
    }

    std::swap(newvals, values);

    if (!multiple_ && values.size() > 1) {
        throw eckit::UserError("Only one value possible for '" + name_ + "'");
    }
}

void Type::setDefaults(MarsRequest& request) {
    if (inheritance_) {
        request.setValuesTyped(this, inheritance_.value());
    }
    else {
        bool unset = false;
        for (const auto& unsetContext : unsets_) {
            if (unsetContext->matches(request)) {
                unset = true;
                break;
            }
        }
        if (!unset) {
            for (const auto& [defaultContext, values] : defaults_) {
                if (defaultContext->matches(request)) {
                    request.setValuesTyped(this, values);
                    break;
                }
            }
        }
    }
}

void Type::setInheritance(const std::vector<std::string>& inheritance) {
    inheritance_ = inheritance;
}

const std::vector<std::string>& Type::flattenValues(const MarsRequest& request) {
    return request.values(name_);
}

void Type::clearDefaults() {
    defaults_.clear();
}

void Type::reset() {
    inheritance_.reset();
}

const std::string& Type::name() const {
    return name_;
}

const std::string& Type::category() const {
    return category_;
}

void Type::pass2(const MarsExpandContext& ctx, MarsRequest& request) {}

void Type::finalise(const MarsExpandContext& ctx, MarsRequest& request, bool strict) {

    const std::vector<std::string>& values = request.values(name_, true);
    if (values.size() == 1 && values[0] == "off") {
        request.unsetValues(name_);
    }
    else {
        if (values.size() > 0) {
            bool matches = (only_.size() == 0);
            for (const auto& context : only_) {
                if (context->matches(request)) {
                    matches = true;
                    break;
                }
            }
            if (!matches) {
                std::ostringstream oss;
                oss << *this << ": Key [" << name_ << "] not acceptable with contextes: " << std::endl;
                for (const auto& context : only_) {
                    oss << "    " << *context << std::endl;
                }
                throw eckit::UserError(oss.str());
            }
            for (const auto& context : unsets_) {
                if (context->matches(request)) {
                    if (strict && request.has(name_)) {
                        std::ostringstream oss;
                        oss << *this << ": Key [" << name_ << "] not acceptable with context: " << *context;
                        throw eckit::UserError(oss.str());
                    }
                    request.unsetValues(name_);
                }
            }
        }

        if (request.verb() != "list") {
            for (const auto& [context, values] : sets_) {
                if (context->matches(request)) {
                    if (strict && !request.has(name_)) {
                        std::ostringstream oss;
                        oss << *this << ": missing Key [" << name_ << "] - required with context: " << *context;
                        throw eckit::UserError(oss.str());
                    }
                    request.setValuesTyped(this, values);
                }
            }
        }
    }
}

void Type::check(const MarsExpandContext& ctx, const std::vector<std::string>& values) const {
    if (flatten_) {
        std::set<std::string> s(values.begin(), values.end());
        if (values.size() != s.size()) {
            std::cerr << "Duplicate values in " << name_ << " " << values;
            std::set<std::string> seen;
            for (const std::string& val : values) {
                if (seen.find(val) != seen.end()) {
                    std::cerr << ' ' << val;
                }

                seen.insert(val);
            }

            std::cerr << std::endl;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
