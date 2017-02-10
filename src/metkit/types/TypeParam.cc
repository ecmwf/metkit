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

static eckit::Mutex *local_mutex = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static std::map<std::string, std::string> *m = 0;

static void init() {

    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, std::string>();

    eckit::PathName language("~metkit/etc/param.json");

    std::ifstream in(language.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(language);
    }

    eckit::JSONParser parser(in);

    const eckit::Value values = parser.parse();
    std::map<std::string, std::string>& mapping = *m;

    for (size_t i = 0; i < values.size(); ++i) {

        const eckit::Value& val = values[i];

        ASSERT (val.isList()) ;
        ASSERT(val.size() > 0);

        std::string first = val[0];

        for (size_t j = 0; j < val.size(); ++j) {
            std::string v = val[j];

            if (mapping.find(v) != mapping.end()) {
                std::cerr << "Redefined param '" << v << "', '" << first << "' and '" << mapping[v] << "'" << std::endl;
                continue;
            }

            mapping[v] = first;
        }
    }
}

static const std::string& lookup(const std::string & s, bool fail) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::map<std::string, std::string>& mapping = *m;

    std::map<std::string, std::string>::const_iterator j = mapping.find(s);
    if (j != mapping.end()) {
        return (*j).second;
    }

    std::string low = eckit::StringTools::lower(s);
    j = mapping.find(low);
    if (j != mapping.end()) {
        mapping[low] = (*j).second;
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
            if(n == &param) {
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

    if(ok && param > 0) {
        std::ostringstream oss;
        oss <<  table * 1000 + param;
        mapping[s] = oss.str();
        return mapping[s];

    }

    if(fail) {
        throw eckit::UserError("Invalid parameter '" + s + "'");
    }

    static std::string empty;
    return empty;
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

bool TypeParam::expand(std::vector<std::string>& values, bool fail) const {


    for (std::vector<std::string>::iterator j = values.begin(); j != values.end(); ++j) {
        std::string& s = (*j);
        s = ::lookup(s, fail);
    }

    return true;
}


void TypeParam::expand(std::vector<std::string>& values) const {
    expand(values, true);
}

void TypeParam::reset() {
    // cache_.clear();
    Type::reset();
}

static TypeBuilder<TypeParam> type("param");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
