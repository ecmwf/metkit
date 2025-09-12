/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/TypeParam.h"

#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/Types.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypesFactory.h"

using eckit::Log;
using metkit::LibMetkit;

namespace {

static eckit::Mutex* local_mutex = 0;
static pthread_once_t once       = PTHREAD_ONCE_INIT;


class Matcher {

    std::string name_;
    eckit::Value values_;

public:

    Matcher(const std::string& name, const eckit::Value values);

    bool match(const metkit::mars::MarsRequest& request, bool partial = false) const;

    friend std::ostream& operator<<(std::ostream& out, const Matcher& matcher) {
        out << matcher.name_ << "=" << matcher.values_;
        return out;
    }
};

Matcher::Matcher(const std::string& name, const eckit::Value values) : name_(name), values_(values) {

    if (!values_.isList()) {
        values_ = eckit::Value::makeList(values_);
    }
}

bool Matcher::match(const metkit::mars::MarsRequest& request, bool partial) const {

    std::vector<std::string> vals = request.values(name_, true);
    if (vals.size() == 0) {
        return partial;
    }

    for (size_t i = 0; i < values_.size(); i++) {
        std::string v = values_[i];

        if (v == vals[0]) {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------

class Rule : public metkit::mars::MarsExpandContext {

    std::vector<Matcher> matchers_;

    std::vector<std::string> values_;
    mutable std::map<std::string, std::string> mapping_;

    static std::vector<std::string> defaultValues_;
    static std::map<std::string, std::string> defaultMapping_;

public:

    bool match(const metkit::mars::MarsRequest& request, bool partial = false) const;
    std::string lookup(const MarsExpandContext& ctx, const std::string& s, bool fail) const;
    long toParamid(const std::string& param) const;

    Rule(const eckit::Value& matchers, const eckit::Value& setters, const eckit::Value& ids);
    static void setDefault(const eckit::Value& setters, const eckit::Value& ids);

    void print(std::ostream& out) const {
        out << "{";
        const char* sep = "";
        for (std::vector<Matcher>::const_iterator j = matchers_.begin(); j != matchers_.end(); ++j) {
            out << sep << (*j);
            sep = ",";
        }
        out << "}";
    }

    void info(std::ostream& out) const {
        out << " ";
        print(out);
    }

    friend std::ostream& operator<<(std::ostream& out, const Rule& rule) {
        rule.print(out);
        return out;
    }
};

std::vector<std::string> Rule::defaultValues_;
std::map<std::string, std::string> Rule::defaultMapping_;

void Rule::setDefault(const eckit::Value& values, const eckit::Value& ids) {

    std::map<std::string, size_t> precedence;

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& id = values[i];

        std::string first = id;
        defaultValues_.push_back(first);

        const eckit::Value& aliases = ids[id];

        if (aliases.isNil()) {

            LOG_DEBUG_LIB(LibMetkit) << "No aliases for " << id << std::endl;
            continue;
        }


        for (size_t j = 0; j < aliases.size(); ++j) {
            std::string v = aliases[j];

            if (defaultMapping_.find(v) != defaultMapping_.end()) {

                if (precedence[v] <= j) {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition ignored: param " << v << "='" << first << "', keeping previous value of '"
                        << defaultMapping_[v] << "' " << std::endl;
                    continue;
                }
                else {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition of param " << v << "='" << first << "', overriding previous value of '"
                        << defaultMapping_[v] << "' " << std::endl;

                    precedence[v] = j;
                }
            }
            else {
                precedence[v] = j;
            }

            defaultMapping_[v] = first;
            defaultValues_.push_back(v);
        }
    }
}

Rule::Rule(const eckit::Value& matchers, const eckit::Value& values, const eckit::Value& ids) {

    std::map<std::string, size_t> precedence;

    const eckit::Value& keys = matchers.keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string name = keys[i];
        matchers_.push_back(Matcher(name, matchers[name]));
    }

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& id = values[i];

        std::string first = id;
        values_.push_back(first);

        const eckit::Value& aliases = ids[id];

        if (aliases.isNil()) {

            LOG_DEBUG_LIB(LibMetkit) << "No aliases for " << id << " " << *this << std::endl;
            continue;
        }


        for (size_t j = 0; j < aliases.size(); ++j) {
            std::string v = aliases[j];

            if (mapping_.find(v) != mapping_.end()) {

                if (precedence[v] <= j) {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition ignored: param " << v << "='" << first << "', keeping previous value of '"
                        << mapping_[v] << "' " << *this << std::endl;
                    continue;
                }
                else {

                    LOG_DEBUG_LIB(LibMetkit)
                        << "Redefinition of param " << v << "='" << first << "', overriding previous value of '"
                        << mapping_[v] << "' " << *this << std::endl;

                    precedence[v] = j;
                }
            }
            else {
                precedence[v] = j;
            }

            mapping_[v] = {first};
            values_.push_back(v);
        }
    }
}


bool Rule::match(const metkit::mars::MarsRequest& request, bool partial) const {
    for (std::vector<Matcher>::const_iterator j = matchers_.begin(); j != matchers_.end(); ++j) {
        if (!(*j).match(request, partial)) {
            return false;
        }
    }
    return true;
}

class ChainedContext : public metkit::mars::MarsExpandContext {
    const MarsExpandContext& ctx1_;
    const MarsExpandContext& ctx2_;

    void info(std::ostream& out) const {
        out << ctx1_;
        out << ctx2_;
        // out << ctx1_ << ctx2_;
    }


public:

    ChainedContext(const MarsExpandContext& ctx1, const MarsExpandContext& ctx2) : ctx1_(ctx1), ctx2_(ctx2) {}
};


std::string Rule::lookup(const MarsExpandContext& ctx, const std::string& s, bool fail) const {

    size_t table = 0;
    size_t param = 0;
    size_t* n    = &param;
    bool ok      = true;

    for (std::string::const_iterator k = s.begin(); k != s.end(); ++k) {
        switch (*k) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                (*n) *= 10;
                (*n) += (*k) - '0';
                break;

            case '.':
                if (n == &param) {
                    n = &table;
                }
                else {
                    ok = false;
                }
                break;

            default:
                ok = false;
                break;
        }
    }

    if (ok && param > 0) {
        std::ostringstream oss;
        if (table == 128) {
            table = 0;
        }

        oss << table * 1000 + param;

        std::string p = oss.str();
        for (std::vector<std::string>::const_iterator j = values_.begin(); j != values_.end(); ++j) {
            if ((*j) == p) {
                return p;
            }
        }

        for (std::vector<std::string>::const_iterator j = defaultValues_.begin(); j != defaultValues_.end(); ++j) {
            if ((*j) == p) {
                return p;
            }
        }

        throw eckit::UserError("Cannot match parameter " + p);
        return p;
    }

    ChainedContext c(ctx, *this);

    std::string paramid = metkit::mars::MarsLanguage::bestMatch(c, s, values_, false, false, true, mapping_);
    if (!paramid.empty()) {
        return paramid;
    }

    return metkit::mars::MarsLanguage::bestMatch(c, s, defaultValues_, fail, false, false, defaultMapping_);
}

static std::deque<Rule>* rules = nullptr;

}  // namespace

static void init() {

    local_mutex = new eckit::Mutex();
    rules       = new std::deque<Rule>();

    const eckit::Value ids = eckit::YAMLParser::decodeFile(LibMetkit::paramIDYamlFile());
    ASSERT(ids.isOrderedMap());

    eckit::ValueMap merge;

    static bool metkitLegacyParamCheck =
        eckit::Resource<bool>("metkitLegacyParamCheck;$METKIT_LEGACY_PARAM_CHECK", false);
    static bool metkitRawParam = eckit::Resource<bool>("metkitRawParam;$METKIT_RAW_PARAM", false);

    if (metkitLegacyParamCheck || (!metkitRawParam)) {
        eckit::Value r = eckit::YAMLParser::decodeFile(LibMetkit::paramYamlFile());
        ASSERT(r.isList());

        const eckit::Value rs = eckit::YAMLParser::decodeFile(LibMetkit::paramStaticYamlFile());
        ASSERT(rs.isList());

        // merge r and rs
        for (size_t i = 0; i < r.size(); ++i) {
            const eckit::Value& rule = r[i];

            if (!rule.isList()) {
                rule.dump(Log::error()) << std::endl;
            }
            ASSERT(rule.isList());
            ASSERT(rule.size() == 2);

            merge.emplace(rule[0], rule[1]);
        }

        for (size_t i = 0; i < rs.size(); ++i) {
            const eckit::Value& rule = rs[i];

            if (!rule.isList()) {
                rule.dump(Log::error()) << std::endl;
            }
            ASSERT(rule.isList());
            ASSERT(rule.size() == 2);

            auto it = merge.find(rule[0]);
            if (it == merge.end()) {
                merge.emplace(rule[0], rule[1]);
            }
            else {
                it->second += rule[1];
            }
        }
    }

    if (metkitLegacyParamCheck) {
        for (auto it = merge.begin(); it != merge.end(); it++) {
            (*rules).push_back(Rule(it->first, it->second, ids));
        }
        return;
    }

    auto keys = ids.keys();
    Rule::setDefault(keys, ids);

    if (metkitRawParam) {
        // empty rule, to enable default
        (*rules).push_back(Rule(eckit::Value::makeMap(), eckit::Value::makeList(), eckit::Value::makeMap()));
        return;
    }

    std::set<std::string> shortnames;
    std::set<std::string> associatedIDs;

    const eckit::Value pc = eckit::YAMLParser::decodeFile(LibMetkit::shortnameContextYamlFile());
    ASSERT(pc.isList());

    for (size_t i = 0; i < pc.size(); i++) {
        shortnames.emplace(pc[i]);
    }

    for (size_t i = 0; i < keys.size(); i++) {
        auto el = ids.element(keys[i]);
        for (size_t j = 0; j < el.size(); j++) {
            if (shortnames.find(el[j]) != shortnames.end()) {
                associatedIDs.emplace(keys[i]);
            }
        }
    }

    for (auto it = merge.begin(); it != merge.end(); it++) {
        auto listIDs = eckit::Value::makeList();

        for (size_t j = 0; j < it->second.size(); j++) {
            if (associatedIDs.find(it->second[j]) != associatedIDs.end()) {
                listIDs.append(it->second[j]);
            }
        }
        if (listIDs.size() > 0) {
            (*rules).push_front(Rule{it->first, listIDs, ids});
        }
    }

    (*rules).push_back(Rule{eckit::Value::makeMap(), eckit::Value::makeList(), eckit::Value::makeMap()});
}

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeParam::TypeParam(const std::string& name, const eckit::Value& settings) : Type(name, settings), firstRule_(false) {

    if (settings.contains("expand_with")) {
        expandWith_ = settings["expand_with"];
    }

    if (settings.contains("first_rule")) {
        firstRule_ = settings["first_rule"];
    }
}

void TypeParam::print(std::ostream& out) const {
    out << "TypeParam[name=" << name_ << "]";
}

void TypeParam::pass2(const MarsExpandContext& ctx, MarsRequest& request) {

    pthread_once(&once, init);

    bool fail                       = true;
    const Rule* rule                = 0;
    std::vector<std::string> values = request.values(name_, true);

    if (values.size() == 1 && values[0] == "all") {
        return;
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    for (std::deque<Rule>::const_iterator j = rules->begin(); j != rules->end(); ++j) {
        if ((*j).match(request)) {
            rule = &(*j);
            break;
        }
    }

    if (!rule) {
        Log::warning() << "TypeParam: cannot find a context to expand 'param' in " << request << std::endl;

        if (firstRule_) {
            bool found = false;
            for (std::deque<Rule>::const_iterator j = rules->begin(); j != rules->end() && !rule; ++j) {
                const Rule* r = &(*j);
                if ((*j).match(request, true)) {
                    for (std::vector<std::string>::iterator j = values.begin(); j != values.end() && !rule; ++j) {
                        std::string& s = (*j);
                        try {
                            s    = r->lookup(ctx, s, fail);
                            rule = r;
                            Log::warning() << "TypeParam: using 'first matching rule' option " << *rule << std::endl;
                        }
                        catch (...) {
                        }
                    }
                }
            }
        }
        else if (expandWith_.size()) {
            MarsRequest tmp(request);
            for (auto j = expandWith_.begin(); j != expandWith_.end(); ++j) {
                if (!tmp.has((*j).first)) {
                    tmp.setValue((*j).first, (*j).second);
                }
            }
            for (std::deque<Rule>::const_iterator j = rules->begin(); j != rules->end(); ++j) {
                if ((*j).match(tmp)) {
                    rule = &(*j);
                    Log::warning() << "TypeParam using 'expand with' option " << *rule << std::endl;
                    break;
                }
            }
        }
        if (!rule) {
            std::ostringstream oss;
            oss << "TypeParam: cannot find a context to expand 'param' in " << request;
            throw eckit::SeriousBug(oss.str());
        }
    }


    for (std::vector<std::string>::iterator j = values.begin(); j != values.end(); ++j) {
        std::string& s = (*j);
        try {
            s = rule->lookup(ctx, s, fail);
        }
        catch (...) {
            Log::error() << *rule << std::endl;
            throw;
        }
    }

    request.setValuesTyped(this, values);
}

bool TypeParam::expand(const MarsExpandContext&, std::string&, const MarsRequest&) const {
    // Work done on pass2()
    return true;
}

void TypeParam::reset() {
    // cache_.clear();
    Type::reset();
}

static TypeBuilder<TypeParam> type("param");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
