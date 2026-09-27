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

#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/ContextRule.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

bool ContextRule::operator<(const ContextRule& other) const {
    std::ostringstream out;
    std::ostringstream outOther;
    out << *this;
    outOther << other;
    return out.str() < outOther.str();
}
bool ContextRule::operator==(const ContextRule& other) const {
    std::ostringstream out;
    std::ostringstream outOther;
    out << *this;
    outOther << other;
    return out.str() == outOther.str();
}

std::unique_ptr<ContextRule> ContextRule::parse(MemFile& file) {
    char type   = static_cast<char>(file.read8());
    Keyword key = file.read16();
    switch (type) {
        case 'I':
            return std::make_unique<Include>(key, file.readStringSet());
        case 'E':
            return std::make_unique<Exclude>(key, file.readStringSet());
        case 'U':
            return std::make_unique<Undef>(key);
        case 'D':
            return std::make_unique<Def>(key);
        default:
            // Handle error or unknown type
            throw eckit::Exception("Unknown ContextRule type");
    }

    // Implementation of parsing logic goes here
}


bool Include::matches(const MarsRequest& req) const {
    static const Keyword verbKey = MarsLanguage::keyword("_verb");
    if (key_ == verbKey) {
        return (vals_.find(req.verb()) != vals_.end());
    }
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

void Include::write(std::ofstream& file) const {
    write8(file, static_cast<uint8_t>('I'));
    write16(file, key_);
    write8(file, vals_.size());
    for (const auto& v : vals_) {
        writeString(file, v);
    }
}

bool Exclude::matches(const MarsRequest& req) const {
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

void Exclude::write(std::ofstream& file) const {
    write8(file, static_cast<uint8_t>('E'));
    write16(file, key_);
    write8(file, vals_.size());
    for (const auto& v : vals_) {
        writeString(file, v);
    }
}

bool Undef::matches(const MarsRequest& req) const {
    return !req.has(key_);
}

void Undef::write(std::ofstream& file) const {
    write8(file, static_cast<uint8_t>('U'));
    write16(file, key_);
}

bool Def::matches(const MarsRequest& req) const {
    return req.has(key_);
}

void Def::write(std::ofstream& file) const {
    write8(file, static_cast<uint8_t>('D'));
    write16(file, key_);
}

//----------------------------------------------------------------------------------------------------------------------

void Context::add(std::unique_ptr<ContextRule> rule) {
    rules_.push_back(std::move(rule));
}

bool Context::matches(const MarsRequest& req) const {

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

std::unique_ptr<ContextRule> parseRule(std::string name, eckit::Value r) {

    std::set<std::string> vals;

    // context rules may reference a keyword before its own YAML entry has been parsed
    // (parsing order across sections/verbs is not guaranteed), so auto-intern rather
    // than require it to already be registered.
    Keyword key = MarsLanguage::addKeyword(name);

    if (r.isList()) {
        if (r.size() == 0) {
            throw eckit::UserError("Empty list for context rule '" + name + "'");
        }

        bool exclude = (r[0] == "!");
        for (size_t k = exclude ? 1 : 0; k < r.size(); k++) {
            vals.insert(r[k]);
        }
        if (exclude)
            return std::make_unique<Exclude>(key, vals);
        return std::make_unique<Include>(key, vals);
    }
    else {
        ASSERT(r.isString());
        std::string v = r;
        if (v == "undefined") {
            return std::make_unique<Undef>(key);
        }
        else if (v == "defined") {
            return std::make_unique<Def>(key);
        }
    }
    return nullptr;
}

Context::Context(size_t id, const eckit::Value& c) : id_(id) {
    if (c.isMap()) {
        eckit::Value keys = c.keys();

        for (size_t j = 0; j < keys.size(); j++) {
            std::string key = keys[j];
            add(parseRule(key, c[key]));
        }
    }
}

Context::Context(size_t id, MemFile& file) : id_(id) {
    uint8_t numRules = file.read8();
    for (uint8_t i = 0; i < numRules; i++) {
        // read each rule from file
        rules_.push_back(ContextRule::parse(file));
    }
}

// Context::Context(Context&& other) : id_(other.id_), rules_(std::move(other.rules_)) {}


bool Context::operator<(const Context& other) const {
    if (rules_.size() != other.rules_.size()) {
        return rules_.size() < other.rules_.size();
    }
    for (size_t i = 0; i < rules_.size(); i++) {
        if (*rules_[i] < *other.rules_[i]) {
            return true;
        }
        if (*other.rules_[i] < *rules_[i]) {
            return false;
        }
    }
    return false;
}

bool Context::operator==(const Context& other) const {
    if (rules_.size() != other.rules_.size()) {
        return false;
    }
    for (size_t i = 0; i < rules_.size(); i++) {
        if (*rules_[i] < *other.rules_[i]) {
            return false;
        }
        if (*other.rules_[i] < *rules_[i]) {
            return false;
        }
    }
    return true;
}

Keyword Context::maxAxisIndex() const {
    Keyword maxIndex = 0;
    for (const auto& r : rules_) {
        if (r->key() < MarsLanguage::maxDataKeyword()) {
            maxIndex = std::max(maxIndex, r->key());
        }
    }
    return maxIndex;
}

void Context::write(std::ofstream& file) const {
    write8(file, rules_.size());
    for (const auto& rule : rules_) {
        rule->write(file);
    }
}

//----------------------------------------------------------------------------------------------------------------------

Type::Type(const std::string& name, Keyword keyword, const eckit::Value& settings) : typeName_(name), id_(keyword) {

    flags_[0] = settings.contains("flatten") ? bool(settings["flatten"]) : true;
    flags_[1] = settings.contains("multiple") ? bool(settings["multiple"]) : false;
    flags_[2] = settings.contains("duplicates") ? bool(settings["duplicates"]) : true;
    flags_[3] = settings.contains("uppercase") ? bool(settings["uppercase"]) : false;
    flags_[4] = settings.contains("first_rule") ? bool(settings["first_rule"]) : false;
    flags_[5] = false;  // ??????

    category_ = Category::None;
    if (settings.contains("category")) {
        std::string category = settings["category"];
        if (category == "data") {
            category_ = Category::Data;
        }
        else if (category == "derived") {
            category_ = Category::Derived;
        }
        else if (category == "postproc") {
            category_ = Category::PostProc;
        }
        else if (category == "sink") {
            category_ = Category::Sink;
        }
        else {
            std::stringstream ss;
            ss << "Unknown category: " << category << " in Type " << MarsLanguage::name(id_);
            throw eckit::SeriousBug(ss.str());
        }
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

                if (d.contains("context")) {
                    const Context& ctx = MarsLanguage::addContext(d["context"]);
                    defaults_.emplace_back(ctx, vals);
                }
                else {
                    defaults_.emplace_back(MarsLanguage::context(0), vals);
                }
            }
        }
    }
}

Type::Type(const std::string& type, Keyword key, MemFile& file) : typeName_(type), id_(key) {

    // typeName_ and id_ have already been consumed by MarsLanguage (to pick the right builder): they are passed in

    flags_    = file.read8();
    category_ = static_cast<Category>(file.read8());

    // read defaults_
    uint8_t num = file.read8();
    for (uint8_t i = 0; i < num; i++) {
        Keyword contextId = file.read16();
        defaults_.emplace_back(MarsLanguage::context(contextId), file.readStringVector());
    }

    // read sets_
    num = file.read8();
    for (uint8_t i = 0; i < num; i++) {
        Keyword contextId = file.read16();
        sets_.emplace_back(MarsLanguage::context(contextId), file.readStringVector());
    }

    // read unsets_
    num = file.read8();
    for (uint8_t i = 0; i < num; i++) {
        Keyword contextId = file.read16();
        unsets_.emplace_back(MarsLanguage::context(contextId));
    }
}

void Type::write(std::ofstream& file) const {
    writeCommon(file);
    writeToByList(file);
}

void Type::writeToByList(std::ofstream& file) const {
    if (toByList_) {
        toByList_->write(file);
    }
}

void Type::writeCommon(std::ofstream& file) const {
    writeString(file, typeName_);
    write16(file, id_);
    write8(file, flags_.to_ulong());
    write8(file, static_cast<uint8_t>(category_));

    // write defaults_
    write8(file, defaults_.size());
    for (const auto& [ctx, vals] : defaults_) {
        write16(file, ctx.get().id());
        write8(file, vals.size());
        for (const auto& val : vals) {
            writeString(file, val);
        }
    }

    // write sets_
    write8(file, sets_.size());
    for (const auto& [ctx, vals] : sets_) {
        write16(file, ctx.get().id());
        write8(file, vals.size());
        for (const auto& val : vals) {
            writeString(file, val);
        }
    }

    // write unsets_
    write8(file, unsets_.size());
    for (const auto& ctx : unsets_) {
        write16(file, ctx.get().id());
    }
}


void Type::defaults(const Context& context, const std::vector<std::string>& values) {
    defaults_.emplace_back(std::cref(context), values);
}
void Type::set(const Context& context, const std::vector<std::string>& values) {
    sets_.emplace_back(std::cref(context), values);
}
void Type::unset(const Context& context) {
    unsets_.emplace_back(std::cref(context));
}
void Type::patchRequest(MarsRequest& request, const std::vector<std::string>& values) const {
    // Special case: inheritance from another key.
    // If the value is of the form _key, then copy values from that key
    if (values.size() == 1 && values[0][0] == '_') {
        std::string key = values[0].substr(1);
        if (request.has(key)) {
            request.setValuesTyped(this, request.values(key));
        }
    }
    else {
        request.setValuesTyped(this, values);
    }
}

bool Type::flatten() const {
    return flags_[0];
}

bool Type::multiple() const {
    return flags_[1];
}

size_t Type::count(const std::vector<std::string>& values) const {
    return flatten() ? values.size() : 1;
}

bool Type::filter(const std::vector<std::string>& filter, std::vector<std::string>& values) const {
    NotInSet not_in_set(filter);

    values.erase(std::remove_if(values.begin(), values.end(), not_in_set), values.end());

    return !values.empty();
}

bool Type::filter(Keyword keyword, const std::vector<std::string>& f, std::vector<std::string>& values) const {

    if (keyword == id()) {
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

std::string Type::tidy(const std::string& value, const MarsRequest& request) const {
    std::string result = value;
    expand(result, request);
    return result;
}

bool Type::expand(std::string& value, const MarsRequest&) const {
    std::ostringstream oss;
    oss << *this << ":  expand not implemented (" << value << ")";
    throw eckit::SeriousBug(oss.str());
}

void Type::expand(std::vector<std::string>& values, const MarsRequest& request) const {

    if (toByList_ && values.size() > 1) {
        toByList_->expandRanges(values, request);
    }

    std::vector<std::string> newvals;
    std::set<std::string> seen;

    for (std::string& val : values) {
        std::string value = val;
        if (!expand(value, request)) {
            std::ostringstream oss;
            oss << *this << ": cannot expand '" << val << "'";
            throw eckit::UserError(oss.str());
        }
        if (hasGroups()) {
            auto gg = group(value);
            if (gg) {
                for (const auto& v : gg->get()) {
                    if (seen.find(v) == seen.end()) {
                        seen.insert(v);
                        newvals.push_back(v);
                    }
                }
            }
        }
        else {
            if (!flags_[2] && seen.find(value) != seen.end()) {
                std::ostringstream oss;
                oss << *this << ": duplicated value '" << value << "'";
                throw eckit::UserError(oss.str());
            }
            newvals.push_back(value);
        }
    }

    std::swap(newvals, values);

    if (!flags_[1] && values.size() > 1) {
        throw eckit::UserError("Only one value possible for '" + name() + "'");
    }
}

void Type::setDefaults(MarsRequest& request) const {
    bool unset = false;
    for (const auto& ctx : unsets_) {
        if (ctx.get().matches(request)) {
            unset = true;
            break;
        }
    }
    if (!unset) {
        for (const auto& [ctx, values] : defaults_) {
            if (ctx.get().matches(request)) {
                patchRequest(request, values);
                break;
            }
        }
    }
}

const std::vector<std::string>& Type::flattenValues(const MarsRequest& request) const {
    return request.values(name());
}

void Type::clearDefaults() {
    defaults_.clear();
}

Keyword Type::id() const {
    return id_;
}

const std::string& Type::name() const {
    return MarsLanguage::name(id_);
}

const Category& Type::category() const {
    return category_;
}

void Type::pass2(MarsRequest& request) const {}

void Type::finalise(MarsRequest& request, bool strict) const {

    auto nn                                = MarsLanguage::name(id_);
    const std::vector<std::string>& values = request.values(nn, true);
    if (values.size() == 1 && values[0] == "off") {
        request.unsetValues(nn);
    }
    else {
        if (values.size() > 0) {
            for (const auto& ctx : unsets_) {
                if (ctx.get().matches(request)) {
                    if (strict && request.has(nn)) {
                        std::ostringstream oss;
                        oss << *this << ": Key [" << name() << "] not acceptable with context: " << ctx.get();
                        throw eckit::UserError(oss.str());
                    }
                    request.unsetValues(nn);
                }
            }
        }

        if (request.verb() != "list") {
            for (const auto& [ctx, values] : sets_) {
                if (ctx.get().matches(request)) {
                    if (strict && !request.has(nn)) {
                        std::ostringstream oss;
                        oss << *this << ": missing Key [" << name() << "] - required with context: " << ctx.get();
                        throw eckit::UserError(oss.str());
                    }
                    patchRequest(request, values);
                }
            }
        }
    }
}

void Type::check(const std::vector<std::string>& values) const {
    if (flatten()) {
        std::set<std::string> s(values.begin(), values.end());
        if (values.size() != s.size()) {
            std::cerr << "Duplicate values in " << name() << " " << values;
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
