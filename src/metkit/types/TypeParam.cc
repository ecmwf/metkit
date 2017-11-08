/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/parser/StringTools.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/Types.h"

#include "metkit/MarsLanguage.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/types/TypeParam.h"
#include "metkit/types/TypesFactory.h"

using eckit::Log;
using metkit::LibMetkit;

namespace {

static eckit::Mutex *local_mutex = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;



class Matcher {

    std::string name_;
    eckit::Value values_;
public:

    Matcher(const std::string& name,
            const eckit::Value values);

    bool match(const metkit::MarsRequest& request) const ;

    friend std::ostream& operator<<(std::ostream& out, const Matcher& matcher) {
        out << matcher.name_ << "=" << matcher.values_;
        return out;
    }
};

Matcher::Matcher(const std::string& name,
                 const eckit::Value values):
    name_(name),
    values_(values) {

    if (!values_.isList()) {
        values_ = eckit::Value::makeList(values_);
    }

}

bool Matcher::match(const metkit::MarsRequest& request) const {

    std::vector<std::string> vals = request.values(name_, true);
    if (vals.size() == 0) {
        return false;
    }

    // std::cout << vals << std::endl;


    for (size_t i = 0; i < values_.size(); i++) {
        std::string v = values_[i];

        if (v == vals[0]) {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------

class Rule : private metkit::ExpandContext {

    std::vector<Matcher> matchers_;
    std::vector<std::string> values_;

    mutable std::map<std::string, std::string> mapping_;

public:

    bool match(const metkit::MarsRequest& request) const;
    std::string lookup(const std::string & s, bool fail) const;

    Rule(const eckit::Value& matchers, const eckit::Value& setters, const eckit::Value& ids);

    void print(std::ostream& out) const {
        out << "{";
        const char* sep = "";
        for (std::vector<Matcher>::const_iterator j = matchers_.begin(); j != matchers_.end(); ++j) {
            out << sep << (*j);
            sep = ",";
        }
        out << "}";
    }

    friend std::ostream& operator<<(std::ostream& out, const Rule& rule) {
        rule.print(out);
        return out;
    }

};


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

            Log::debug<LibMetkit>()
                      << "No aliases for "
                      << id
                      << " "
                      << *this
                      << std::endl;
            continue;
        }



        for (size_t j = 0; j < aliases.size(); ++j) {
            std::string v = aliases[j];

            if (mapping_.find(v) != mapping_.end()) {

                if (precedence[v] <= j) {

                    Log::debug<LibMetkit>()
                            << "Redefinition ignored: param "
                            << v
                            << "='"
                            << first
                            << "', keeping previous value of '"
                            << mapping_[v]
                               << "' "
                               << *this
                               << std::endl;
                    continue;
                }
                else {

                    Log::debug<LibMetkit>()
                            << "Redefinition of param "
                            << v
                            << "='"
                            << first
                            << "', overriding previous value of '"
                            << mapping_[v]
                               << "' "
                               << *this
                               << std::endl;

                    precedence[v] = j;
                }
            } else {
                precedence[v] = j;
            }

            mapping_[v] = first;
            values_.push_back(v);
        }
    }
}



bool Rule::match(const metkit::MarsRequest& request) const {
    for (std::vector<Matcher>::const_iterator j = matchers_.begin(); j != matchers_.end(); ++j) {
        if (!(*j).match(request)) {
            return false;
        }
    }
    return true;
}


std::string Rule::lookup(const std::string & s, bool fail) const {

    size_t table = 0;
    size_t param = 0;
    size_t *n = &param;
    bool ok = true;


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

        // std::cerr << "Param " << param << " " << table << std::endl;

        oss <<  table * 1000 + param;
        // return  metkit::MarsLanguage::bestMatch(oss.str(), values_, fail, false, mapping_, this);

        std::string p = oss.str();
        for (std::vector<std::string>::const_iterator j = values_.begin(); j != values_.end(); ++j) {
            if ((*j) == p) {
                return p;
            }
        }

        // throw eckit::UserError("Cannot match parameter " + p);
        eckit::Log::warning() << "Cannot match parameter " << p  << std::endl;
        return p;

    }

        // std::cout << "--- [" << s << "]" << std::endl;
        // std::cout << "--- [" << values_ << "]" << std::endl;


    return metkit::MarsLanguage::bestMatch(s, values_, fail, false, mapping_, this);
}

static std::vector<Rule>* rules = 0;

}

static void init() {

    local_mutex = new eckit::Mutex();
    rules = new std::vector<Rule>();

    const eckit::Value ids = eckit::YAMLParser::decodeFile("~metkit/share/metkit/paramids.yaml");
    ASSERT(ids.isOrderedMap());

    const eckit::Value r = eckit::YAMLParser::decodeFile("~metkit/share/metkit/param.yaml");
    ASSERT(r.isList());

    // r.dump(std::cout) << std::endl;

    for (size_t i = 0; i < r.size(); ++i) {
        const eckit::Value& rule = r[i];

        if(!rule.isList()) {
            rule.dump(Log::error()) << std::endl;
        }

        ASSERT(rule.isList());
        ASSERT(rule.size() == 2);

        (*rules).push_back(Rule(rule[0], rule[1], ids));
    }
}



namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeParam::TypeParam(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {



}

TypeParam::~TypeParam() {
}

void TypeParam::print(std::ostream &out) const {
    out << "TypeParam[name=" << name_ << "]";
}

bool TypeParam::expand(const MarsRequest& request, std::vector<std::string>& values, bool fail) const {

    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const Rule* rule = 0;

    for (std::vector<Rule>::const_iterator j = rules->begin(); j != rules->end(); ++j) {
        if ((*j).match(request)) {
            rule = &(*j);
            break;
        }
    }


    if (!rule) {
        Log::error() << "Not rule for " << request << std::endl;
    }
    ASSERT(rule);


    for (std::vector<std::string>::iterator j = values.begin(); j != values.end(); ++j) {
        std::string& s = (*j);
        try {
            s = rule->lookup(s, fail);
        } catch (...) {
            Log::error() << *rule << std::endl;
            throw;
        }
    }

    return true;
}


void TypeParam::pass2(MarsRequest& request) {
    // std::cout << request << std::endl;
    std::vector<std::string> values = request.values(name_, true);
    expand(request, values, true);
    request.setValuesTyped(this, values);
}


void TypeParam::expand(std::vector<std::string>& values) const {
// Work done on pass2()
}

void TypeParam::reset() {
    // cache_.clear();
    Type::reset();
}

static TypeBuilder<TypeParam> type("param");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
