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

#include <unordered_map>

#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/TypesFactory.h"

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>

using eckit::Log;
using metkit::LibMetkit;

using ParamIdAliases = std::unordered_map<uint32_t, std::vector<std::string>>;

namespace {

static eckit::Mutex* local_mutex = 0;
static pthread_once_t once       = PTHREAD_ONCE_INIT;
class Matcher {

    std::string name_;
    std::vector<std::string> values_;

    friend class Rule;

public:

    // Matcher(const std::string& name, const eckit::Value values);
    Matcher(const std::string& name, std::vector<std::string>&& values);

    bool match(const metkit::mars::MarsRequest& request, bool partial = false) const;

    friend std::ostream& operator<<(std::ostream& out, const Matcher& matcher) {
        out << matcher.name_ << "=[";
        std::string separator{};
        for (const auto& v : matcher.values_) {
            out << separator << v;
            separator = ",";
        }
        out << "]";
        return out;
    }
};

Matcher::Matcher(const std::string& name, std::vector<std::string>&& values) :
    name_(name), values_(std::move(values)) {}

bool Matcher::match(const metkit::mars::MarsRequest& request, bool partial) const {

    std::vector<std::string> vals = request.values(name_, true);
    if (vals.size() == 0) {
        return partial;
    }

    for (const auto& v : values_) {
        if (v == vals[0]) {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------

class Rule {

    std::vector<Matcher> matchers_;

    std::unordered_set<uint32_t> values_;
    mutable std::map<std::string, std::string> mapping_;

    static std::unordered_set<uint32_t> defaultValues_;
    static std::map<std::string, std::string> defaultMapping_;

private:

    Rule(std::vector<Matcher>&& matchers, std::unordered_set<uint32_t>&& values,
         std::map<std::string, std::string>&& mapping) :
        matchers_(std::move(matchers)), values_(std::move(values)), mapping_(std::move(mapping)) {}

public:

    static void init();

    bool match(const metkit::mars::MarsRequest& request, bool partial = false) const;
    std::string lookup(const std::string& s) const;
    long toParamid(const std::string& param) const;

    Rule(const eckit::Value& matchers, const eckit::Value& setters, const ParamIdAliases& ids);
    static void setDefault(const eckit::Value& setters, const ParamIdAliases& ids);

    void print(std::ostream& out) const {
        out << "matchers=[";
        std::string sep = "";
        for (const auto& m : matchers_) {
            out << sep << m;
            sep = ",";
        }
        out << "],values=[";
        sep = "";
        for (const auto& v : values_) {
            out << sep << v;
            sep = ",";
        }
        out << "],aliases=[";
        sep = "";
        for (const auto& [s, k] : mapping_) {
            out << sep << s << "->" << k;
            sep = ",";
        }
        out << "]";
    }

    friend std::ostream& operator<<(std::ostream& out, const Rule& rule) {
        rule.print(out);
        return out;
    }
};

static void initRules() {
    Rule::init();
}

std::unordered_set<uint32_t> Rule::defaultValues_;
std::map<std::string, std::string> Rule::defaultMapping_;

void Rule::setDefault(const eckit::Value& values, const ParamIdAliases& ids) {

    std::map<std::string, size_t> precedence;

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& id = values[i];

        std::string first = id;
        defaultValues_.insert(std::stoi(first));

        auto it = ids.find(id);
        if (it == ids.end()) {

            LOG_DEBUG_LIB(LibMetkit) << "No aliases for " << id << std::endl;
            continue;
        }


        for (size_t j = 0; j < it->second.size(); ++j) {
            std::string v = it->second.at(j);

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
        }
    }
}

Rule::Rule(const eckit::Value& matchers, const eckit::Value& values, const ParamIdAliases& ids) {

    static bool multiParamValues = eckit::Resource<bool>("metkitMultiParamValues;$METKIT_MULTI_PARAM_VALUES", false);
    std::map<std::string, size_t> precedence;

    const eckit::Value& keys = matchers.keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string name = keys[i];

        auto rawValues = matchers[name];
        std::vector<std::string> values;
        if (!rawValues.isList()) {
            values.push_back(rawValues);
        }
        else {
            for (size_t i = 0; i < rawValues.size(); i++) {
                std::string v = rawValues[i];
                values.push_back(v);
            }
        }

        matchers_.emplace_back(name, std::move(values));
    }

    bool printed = false;
    std::ostringstream out;
    out << "matchers=[";
    std::string sep = "";
    for (const auto& m : matchers_) {
        out << sep << m;
        sep = ",";
    }
    out << "]";

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& id = values[i];

        std::string first = id;
        uint32_t paramid  = std::stoul(first);
        values_.insert(paramid);

        const auto& aliases = ids.find(paramid)->second;

        if (aliases.size() == 0) {

            LOG_DEBUG_LIB(LibMetkit) << "No aliases for " << id << " " << *this << std::endl;
            continue;
        }


        for (size_t j = 0; j < aliases.size(); ++j) {
            const std::string& v = aliases[j];

            auto it = mapping_.find(v);

            if (it == mapping_.end()) {
                mapping_[v] = first;
            }
            else if (multiParamValues) {
                eckit::Tokenizer tokenizer("|");
                std::vector<std::string> tokens;
                tokenizer(it->second, tokens);
                bool found = false;
                for (const auto& vv : tokens) {
                    if (vv == first) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    it->second = first + "|" + it->second;
                }
            }
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

std::string Rule::lookup(const std::string& s) const {

    size_t table = 0;
    size_t param = 0;
    size_t* n    = &param;
    bool numeric = true;

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
                    numeric = false;
                }
                break;

            default:
                numeric = false;
                break;
        }
    }

    if (numeric && param > 0) {
        if (table == 128) {
            table = 0;
        }

        if (table > 0 && param >= 1000) {
            throw eckit::UserError("Unrecognised format for parameter " + s, Here());
        }

        uint32_t pp = table * 1000 + param;

        auto it = values_.find(pp);
        if (it == values_.end()) {
            it = defaultValues_.find(pp);
            if (it == defaultValues_.end()) {
                std::ostringstream ss;
                ss << "Cannot match parameter " << pp;
                throw eckit::UserError(ss.str(), Here());
            }
        }

        std::ostringstream ss;
        ss << pp;
        return ss.str();
    }

    std::string pp = eckit::StringTools::lower(s);

    // not numeric... check just the aliases
    auto it = mapping_.find(pp);
    if (it == mapping_.end()) {
        it = defaultMapping_.find(pp);
        if (it == defaultMapping_.end()) {
            throw eckit::UserError("Cannot match parameter " + s, Here());
        }
    }
    return it->second;
}

static std::vector<Rule>* rules = nullptr;

}  // namespace

void Rule::init() {

    static bool metkitLegacyParamCheck =
        eckit::Resource<bool>("metkitLegacyParamCheck;$METKIT_LEGACY_PARAM_CHECK", false);
    static bool metkitRawParam   = eckit::Resource<bool>("metkitRawParam;$METKIT_RAW_PARAM", false);
    static bool precomputedParam = eckit::Resource<bool>("metkitPrecomputedParam;$METKIT_PRECOMPUTED_PARAM", true);

    local_mutex = new eckit::Mutex();
    rules       = new std::vector<Rule>();

    if (precomputedParam && !metkitLegacyParamCheck && !metkitRawParam) {
        eckit::PathName paramBinFile = LibMetkit::paramsBinaryFile();
        if (paramBinFile.exists()) {
            size_t numRules;
            size_t size;
            size_t numValues;
            size_t stringSize;
            std::fstream file(paramBinFile.localPath(), std::ios::binary | std::ios::in);

            // read defaultValues_
            file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
            uint32_t v;
            for (size_t i = 0; i < size; i++) {
                file.read(reinterpret_cast<char*>(&v), sizeof(uint32_t));
                defaultValues_.insert(v);
            }

            // read defaultMapping_
            file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
            std::string key;
            std::string value;
            for (size_t i = 0; i < size; i++) {
                file.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
                key.resize(stringSize);
                file.read(key.data(), sizeof(char) * stringSize);
                file.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
                value.resize(stringSize);
                file.read(value.data(), sizeof(char) * stringSize);
                defaultMapping_.emplace(key, value);
            }

            // read rules
            file.read(reinterpret_cast<char*>(&numRules), sizeof(size_t));
            for (size_t ruleIdx = 0; ruleIdx < numRules; ruleIdx++) {
                // read Matchers
                std::vector<Matcher> matchers;
                file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
                for (size_t matcherIdx = 0; matcherIdx < size; matcherIdx++) {
                    std::unordered_set<uint32_t> values;
                    file.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
                    key.resize(stringSize);
                    file.read(key.data(), sizeof(char) * stringSize);
                    file.read(reinterpret_cast<char*>(&numValues), sizeof(size_t));
                    std::vector<std::string> matcherValues;
                    matcherValues.resize(numValues);
                    for (size_t valueIdx = 0; valueIdx < numValues; valueIdx++) {
                        file.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
                        value.resize(stringSize);
                        file.read(value.data(), sizeof(char) * stringSize);
                        matcherValues.push_back(value);
                    }
                    matchers.emplace_back(key, std::move(matcherValues));
                }

                // read values
                std::unordered_set<uint32_t> values;
                file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
                uint32_t v;
                for (size_t valueIdx = 0; valueIdx < size; valueIdx++) {
                    file.read(reinterpret_cast<char*>(&v), sizeof(uint32_t));
                    values.insert(v);
                }

                // read alias mapping
                std::map<std::string, std::string> mapping;
                file.read(reinterpret_cast<char*>(&size), sizeof(size_t));
                for (size_t mapperIdx = 0; mapperIdx < size; mapperIdx++) {
                    file.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
                    key.resize(stringSize);
                    file.read(key.data(), sizeof(char) * stringSize);
                    file.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
                    value.resize(stringSize);
                    file.read(value.data(), sizeof(char) * stringSize);
                    mapping.emplace(key, value);
                }
                auto rule = Rule{std::move(matchers), std::move(values), std::move(mapping)};
                rules->push_back(std::move(rule));
            }
            file.close();

            return;
        }
    }

    const eckit::Value rawIds = eckit::YAMLParser::decodeFile(LibMetkit::paramIDYamlFile());
    ASSERT(rawIds.isOrderedMap());
    auto keys = rawIds.keys();
    ParamIdAliases ids;
    ASSERT(keys.isList());
    for (size_t i = 0; i < keys.size(); ++i) {
        uint32_t id     = keys[i];
        auto rawAliases = rawIds[keys[i]];
        size_t idx      = 0;
        std::vector<std::string> aliases;
        for (size_t j = 0; j < rawAliases.size(); j++) {
            std::string alias = rawAliases[j];
            if (idx != 1 && alias.size() < 20 &&
                alias.find(" ") == -1) {  // short string, no blanks --> it should be a shortname
                aliases.push_back(alias);
            }
            idx++;
        }
        ids.emplace(id, aliases);
    }

    eckit::ValueMap merge;

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

    Rule::setDefault(keys, ids);

    if (metkitRawParam) {
        // empty rule, to enable default
        (*rules).push_back(Rule(eckit::Value::makeMap(), eckit::Value::makeList(), ParamIdAliases{}));
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
        auto el = rawIds.element(keys[i]);
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
            (*rules).push_back(Rule{it->first, listIDs, ids});
        }
    }

    (*rules).push_back(Rule{eckit::Value::makeMap(), eckit::Value::makeList(), ParamIdAliases{}});

    if (precomputedParam && !metkitLegacyParamCheck && !metkitRawParam) {  // creating the binary file
        eckit::PathName paramBinFile = LibMetkit::paramsBinaryFile();
        if (!paramBinFile.exists()) {

            std::fstream file{paramBinFile.localPath(), std::ios::binary | std::ios::out};

            size_t numRules;
            size_t size = Rule::defaultValues_.size();
            size_t numValues;
            size_t stringSize;
            file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
            for (auto v : defaultValues_) {
                file.write(reinterpret_cast<const char*>(&v), sizeof(uint32_t));
            }
            size = Rule::defaultMapping_.size();
            file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
            for (const auto& [name, id] : defaultMapping_) {
                {
                    size_t stringSize = name.size();
                    file.write(reinterpret_cast<const char*>(&stringSize), sizeof(size_t));
                    file.write(name.c_str(), sizeof(char) * stringSize);
                }
                {
                    size_t stringSize = id.size();
                    file.write(reinterpret_cast<const char*>(&stringSize), sizeof(size_t));
                    file.write(id.c_str(), sizeof(char) * stringSize);
                }
            }

            numRules = rules->size();
            file.write(reinterpret_cast<const char*>(&numRules), sizeof(size_t));
            for (const auto& r : *rules) {
                // write matchers
                size = r.matchers_.size();
                file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
                for (const auto& m : r.matchers_) {
                    // std::string name_;
                    size_t stringSize = m.name_.size();
                    file.write(reinterpret_cast<const char*>(&stringSize), sizeof(size_t));
                    file.write(m.name_.c_str(), sizeof(char) * stringSize);

                    // std::vector<std::string> values_;
                    numValues = m.values_.size();
                    file.write(reinterpret_cast<const char*>(&numValues), sizeof(size_t));
                    for (const auto& v : m.values_) {
                        size_t stringSize = v.size();
                        file.write(reinterpret_cast<const char*>(&stringSize), sizeof(size_t));
                        file.write(v.c_str(), sizeof(char) * stringSize);
                    }
                }

                // write values
                size = r.values_.size();
                file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
                for (const auto& v : r.values_) {
                    file.write(reinterpret_cast<const char*>(&v), sizeof(uint32_t));
                }

                // write mapping
                size = r.mapping_.size();
                file.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
                for (const auto& [name, id] : r.mapping_) {
                    size_t stringSize = name.size();
                    file.write(reinterpret_cast<const char*>(&stringSize), sizeof(size_t));
                    file.write(name.c_str(), sizeof(char) * stringSize);
                    stringSize = id.size();
                    file.write(reinterpret_cast<const char*>(&stringSize), sizeof(size_t));
                    file.write(id.c_str(), sizeof(char) * stringSize);
                }
            }
            file.close();
        }
    }
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

void TypeParam::pass2(MarsRequest& request) {

    pthread_once(&once, initRules);

    const Rule* rule                = 0;
    std::vector<std::string> values = request.values(name_, true);

    if (values.size() == 1 && values[0] == "all") {
        return;
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    for (const auto& r : *rules) {
        if (r.match(request)) {
            rule = &r;
            break;
        }
    }

    if (!rule) {
        Log::warning() << "TypeParam: cannot find a context to expand 'param' in " << request << std::endl;

        if (firstRule_) {
            bool found = false;
            for (const auto& r : *rules) {
                if (r.match(request, true)) {
                    for (std::vector<std::string>::iterator j = values.begin(); j != values.end() && !rule; ++j) {
                        std::string& s = (*j);
                        try {
                            s    = r.lookup(s);
                            rule = &r;
                            Log::warning() << "TypeParam: using 'first matching rule' option " << r << std::endl;
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
            for (const auto& r : *rules) {
                if (r.match(tmp)) {
                    rule = &r;
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
            s = rule->lookup(s);
        }
        catch (...) {
            Log::error() << *rule << std::endl;
            throw;
        }
    }

    request.setValuesTyped(this, values);
}

bool TypeParam::expand(std::string&, const MarsRequest&) const {
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
