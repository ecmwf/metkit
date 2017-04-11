/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeParam.h"
#include "metkit/MarsLanguage.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/types/Types.h"
#include "eckit/parser/StringTools.h"

#include "eckit/thread/AutoLock.h"

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

    for (size_t i = 0; i < values_.size(); i++) {
        std::string v = values_[i];
        std::vector<std::string> vals = request.values(name_, true);

        std::cout << name_ << " " << vals << " " << v << std::endl;

        if (vals.size() == 0) {
            return false;
        }

        return (v == vals[0]);

    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------

class Rule {

    std::vector<Matcher> matchers_;
    mutable std::map<std::string, std::string> mapping_;

public:

    bool match(const metkit::MarsRequest& request) const;
    const std::string& lookup(const std::string & s, bool fail) const;

    Rule(const eckit::Value& matchers, const eckit::Value& setters);
};


Rule::Rule(const eckit::Value& matchers, const eckit::Value& values) {

    const eckit::Value& keys = matchers.keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string name = keys[i];
        matchers_.push_back(Matcher(name, matchers[name]));
    }

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& val = values[i];

        ASSERT(val.isList()) ;
        ASSERT(val.size() > 0);

        std::string first = val[0];

        for (size_t j = 0; j < val.size(); ++j) {
            std::string v = val[j];

            if (mapping_.find(v) != mapping_.end()) {
                std::cerr << "Redefined param '" << v << "', '" << first << "' and '" << mapping_[v] << "'" << std::endl;
                continue;
            }

            mapping_[v] = first;
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


const std::string& Rule::lookup(const std::string & s, bool fail) const {



    std::map<std::string, std::string>::const_iterator j = mapping_.find(s);
    if (j != mapping_.end()) {
        return (*j).second;
    }

    std::string low = eckit::StringTools::lower(s);
    j = mapping_.find(low);
    if (j != mapping_.end()) {
        mapping_[low] = (*j).second;
        return (*j).second;
    }

    size_t table = 0;
    size_t param = 0;
    size_t *n = &param;
    bool ok = true;

    for (std::string::const_iterator k = low.begin(); k != low.end(); ++k) {
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
        oss <<  table * 1000 + param;
        mapping_[s] = oss.str();
        return mapping_[s];

    }

    if (fail) {
        throw eckit::UserError("Invalid parameter '" + s + "'");
    }

    static std::string empty;
    return empty;
}

static std::vector<Rule>* rules = 0;

}

static void init() {

    local_mutex = new eckit::Mutex();
    rules = new std::vector<Rule>();

    eckit::PathName language("~metkit/etc/param.json");

    std::ifstream in(language.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(language);
    }

    eckit::JSONParser parser(in);

    const eckit::Value r = parser.parse();
    ASSERT(r.isList());

    for (size_t i = 0; i < r.size(); ++i) {
        const eckit::Value& rule = r[i];
        ASSERT(rule.isList());
        ASSERT(rule.size() == 2);
        (*rules).push_back(Rule(rule[0], rule[1]));
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

    ASSERT(rule);

    for (std::vector<std::string>::iterator j = values.begin(); j != values.end(); ++j) {
        std::string& s = (*j);
        s = rule->lookup(s, fail);
    }

    return true;
}


void TypeParam::expand(const MarsRequest& request, std::vector<std::string>& values) const {
    expand(request, values, true);
}

void TypeParam::reset() {
    // cache_.clear();
    Type::reset();
}

static TypeBuilder<TypeParam> type("param");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
