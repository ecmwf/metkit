/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <set>
#include <list>

#include "eckit/types/Types.h"
#include "eckit/parser/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/config/Resource.h"
#include "eckit/utils/Translator.h"
#include "eckit/utils/MD5.h"
#include "eckit/parser/StringTools.h"
#include "eckit/memory/ScopedPtr.h"

#include "metkit/MarsLanguage.h"
#include "eckit/parser/JSONParser.h"
#include "metkit/types/TypesFactory.h"
#include "metkit/types/Type.h"

using namespace eckit;


static pthread_once_t once = PTHREAD_ONCE_INIT;
static Value languages;

static void init() {

    eckit::PathName language("~metkit/etc/language.json");
    std::cout << "Loading " << language << std::endl;
    std::ifstream in(language.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(language);
    }

    eckit::JSONParser parser(in);

    languages =  parser.parse();

}



namespace metkit {

MarsLanguage::MarsLanguage(const std::string& verb):
    verb_(verb) {
    pthread_once(&once, init);
    lang_ = languages[verb];
}

static std::string bestMatch(const std::string& name, const Value& values) {
    for (size_t i = 0; i < values.size(); ++i) {
        std::string value = values[i];
        if (value.find(name) == 0) {
            return value;
        }
    }

    std::ostringstream oss;
    oss << "Cannot match '" << name << "' in " << values;
    throw eckit::UserError(oss.str());
}

std::string MarsLanguage::expandVerb(const std::string& verb) {
    pthread_once(&once, init);

    return bestMatch(verb, languages.keys());

}

std::string MarsLanguage::expandKeyword(const std::string& keyword) {
    return bestMatch(keyword, lang_.keys());
}

void MarsLanguage::expandValues(const std::string& keyword,
                                const Value& language,
                                std::vector<std::string>& values) const {

    eckit::ScopedPtr<Type> type(TypesFactory::build(language["type"], keyword, language));
    type->expand(values);
}

MarsRequest MarsLanguage::expand(const MarsRequest& r) const {
    MarsRequest result(verb_);

    std::vector<std::string> params;
    r.getParams(params);

    for (std::vector<std::string>::iterator j = params.begin(); j != params.end(); ++j) {
        std::string p = bestMatch(*j, lang_.keys());

        std::vector<std::string> values;
        r.getValues(*j, values);

        expandValues(p, lang_[p], values);

        result.setValues(p, values);
    }

    return result;
}


const std::string& MarsLanguage::verb() const {
    return verb_;
}


//----------------------------------------------------------------------------------------------------------------------
void MarsLanguage::set(const std::string& name, const std::vector<std::string>& values) {
    inheritence_[name] = values;
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
