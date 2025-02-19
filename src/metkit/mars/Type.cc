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

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

ContextRule::ContextRule(const std::string& k) : key_(k) {}

std::ostream& operator<<(std::ostream& s, const ContextRule& r) {
    r.print(s);
    return s;
}

//----------------------------------------------------------------------------------------------------------------------

void Context::add(ContextRule* rule) {
    rules_.emplace_back(rule);
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

class Include : public ContextRule {
public:
    Include(const std::string& k, const std::set<std::string>& vv) : ContextRule(k), vals_(vv) {}

    bool matches(MarsRequest req) const override {
        if (!req.has(key_)) {
            return false;
        }
        for (const std::string& v : req.values(key_)) {
            if (vals_.find(v) != vals_.end()) {
                return true;
            }
        }
        return false;
    }
private:  // methods
    void print(std::ostream& out) const override {
        out << "Include[key=" << key_ << ",vals=[" << vals_ << "]]";
    }
private:
    std::set<std::string> vals_;
};

class Exclude : public ContextRule {
public:
    Exclude(const std::string& k, const std::set<std::string>& vv) : ContextRule(k), vals_(vv) {}
    bool matches(MarsRequest req) const override {
        if (!req.has(key_)) {
            return false;
        }
        for (const std::string& v : req.values(key_)) {
            if (vals_.find(v) != vals_.end()) {
                return false;
            }
        }
        return true;
    }
private:  // methods
    void print(std::ostream& out) const override {
        out << "Exclude[key=" << key_ << ",vals=[" << vals_ << "]]";
    }
private:
    std::set<std::string> vals_;
};

class Undef : public ContextRule {
public:
    Undef(const std::string& k) : ContextRule(k) {}
    bool matches(MarsRequest req) const override {
        return !req.has(key_);
    }
private:  // methods
    void print(std::ostream& out) const override {
        out << "Undef[key=" << key_ << "]";
    }
};

class Def : public ContextRule {
public:
    Def(const std::string& k) : ContextRule(k) {}
    bool matches(MarsRequest req) const override {
        return req.has(key_);
    }
private:  // methods
    void print(std::ostream& out) const override {
        out << "Def[key=" << key_ << "]";
    }
};

//----------------------------------------------------------------------------------------------------------------------
// HELPERS

namespace {

ContextRule* parseRule(std::string key, eckit::Value r) {

    std::set<std::string> vals;

    if (r.isList()) {
        for (size_t k = 0; k < r.size(); k++) {
            vals.insert(r[k]);
        }
        return new Include(key, vals);
    }

    ASSERT(r.contains("op"));
    std::string op = r["op"];
    ASSERT(op.size() == 1);
    switch (op[0]) {
        case 'u': return new Undef(key);
        case 'd': return new Def(key);
        case '!':
            ASSERT(r.contains("vals"));
            eckit::Value vv = r["vals"];
            for (size_t k = 0; k < vv.size(); k++) {
                vals.insert(vv[k]);
            }
            return new Exclude(key, vals);
    }
    return nullptr;
}

Context* parseContext(eckit::Value c) {

    Context* context = new Context{};

    eckit::Value keys = c.keys();

    for (size_t j = 0; j < keys.size(); j++) {
        std::string key = keys[j];
        ContextRule* r  = parseRule(key, c[key]);
        context->add(r);
    }
    return context;
}

}  // namespace

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
                ASSERT(d.contains("vals"));
                eckit::Value vv = d["vals"];

                if (vv.isList()) {
                    for (size_t k = 0; k < vv.size(); k++) {
                        vals.push_back(vv[k]);
                    }
                }
                else {
                    vals.push_back(vv);
                }

                Context* context = nullptr;
                if (d.contains("context")) {
                    context = parseContext(d["context"]);
                } else {
                    context = new Context;
                }
                defaults_.emplace(context, vals);
            }
        }
    }
    if (settings.contains("set")) {
        eckit::Value sets = settings["set"];
        if (!sets.isNil() && sets.isList()) {
            for (size_t i = 0; i < sets.size(); i++) {
                eckit::Value s = sets[i];
                ASSERT(s.contains("val"));
                eckit::Value val = s["val"];
                ASSERT(s.contains("context"));
                sets_.emplace(parseContext(s["context"]), val);
            }
        }
    }
    if (settings.contains("unset")) {
        eckit::Value unsets = settings["unset"];
        if (!unsets.isNil() && unsets.isList()) {
            for (size_t i = 0; i < unsets.size(); i++) {
                eckit::Value u = unsets[i];
                ASSERT(u.contains("context"));
                Context* c = parseContext(u["context"]);
                unsets_.emplace(c);
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

class InSet {
    std::set<std::string> set_;

public:
    InSet(const std::vector<std::string>& f) : set_(f.begin(), f.end()) {}

    bool operator()(const std::string& s) const { return set_.find(s) != set_.end(); }
};

bool Type::matches(const std::vector<std::string>& match,
                   const std::vector<std::string>& values) const {
    InSet in_set(match);
    return std::find_if(values.begin(), values.end(), in_set) != values.end();
}


std::ostream& operator<<(std::ostream& s, const Type& x) {
    x.print(s);
    return s;
}


std::string Type::tidy(const MarsExpandContext& ctx, const std::string& value) const {
    std::string result = value;
    expand(ctx, result);
    return result;
}

std::string Type::tidy(const std::string& value) const {
    DummyContext ctx;
    return tidy(ctx, value);
}

std::vector<std::string> Type::tidy(const std::vector<std::string>& values) const {
    DummyContext ctx;

    std::vector<std::string> result;
    result.reserve(values.size());

    std::transform(values.begin(), values.end(), std::back_inserter(result),
                   [this, ctx](const std::string& s) { return this->tidy(ctx, s); }
                   );

    return result;
}

bool Type::expand(const MarsExpandContext&, std::string& value) const {
    std::ostringstream oss;
    oss << *this << ":  expand not implemented (" << value << ")";
    throw eckit::SeriousBug(oss.str());
}

void Type::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {
    std::vector<std::string> newvals;

    std::set<std::string> seen;

    for (const std::string& val : values) {
        std::string value = val;
        if (!expand(ctx, value)) {
            std::ostringstream oss;
            oss << *this << ": cannot expand '" << val << "'" << ctx;
            throw eckit::UserError(oss.str());
        }

        if (!duplicates_) {
            if (seen.find(value) != seen.end()) {
                std::ostringstream oss;
                oss << *this << ": duplicated value '" << val << "'" << ctx;
                throw eckit::UserError(oss.str());
            }
            seen.insert(value);
        }

        newvals.push_back(value);
    }

    std::swap(newvals, values);

    if (!multiple_ && values.size() > 1) {
        throw eckit::UserError("Only one value passible for '" + name_ + "'");
    }
}

void Type::setDefaults(MarsRequest& request) {
    if (inheritance_) {
        request.setValuesTyped(this, inheritance_.value());
    } else {
        for (const auto& [context, values] : defaults_) {
            if (context->matches(request)) {
                request.setValuesTyped(this, values);
                break;
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
        for (const auto& context : unsets_) {
            if (context->matches(request)) {
                request.unsetValues(name_);
            }
        }

        for (const auto& [context, value]: sets_) {
            if (context->matches(request)) {
                request.setValuesTyped(this, std::vector<std::string>{value});
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
