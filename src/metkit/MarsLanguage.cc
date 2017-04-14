/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "metkit/MarsExpension.h"
#include "eckit/log/Timer.h"

using namespace eckit;


static pthread_once_t once = PTHREAD_ONCE_INIT;
static Value languages_;
static std::vector<std::string> verbs_;

static void init() {

    eckit::PathName language("~metkit/etc/language.json");

    std::ifstream in(language.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(language);
    }

    eckit::JSONParser parser(in);

    languages_ =  parser.parse();
    Value verbs = languages_.keys();
    for (size_t i = 0; i < verbs.size(); ++i) {
        verbs_.push_back(verbs[i]);
    }

}



namespace metkit {

MarsLanguage::MarsLanguage(const std::string& verb):
    verb_(verb) {
    pthread_once(&once, init);
    Value lang = languages_[verb];
    Value params = lang.keys();

    for (size_t i = 0; i < params.size(); ++i) {
        std::string keyword = params[i];
        Value settings = lang[keyword];
        types_[keyword] = TypesFactory::build(keyword, settings);
        types_[keyword]->attach();
        keywords_.push_back(keyword);

        if (settings.contains("aliases")) {
            Value aliases = settings["aliases"];
            for (size_t j = 0; j < aliases.size(); ++j) {
                aliases_[aliases[j]] = keyword;
                keywords_.push_back(aliases[j]);
            }
        }

    }
}

MarsLanguage::~MarsLanguage() {
    for (std::map<std::string, Type* >::iterator j = types_.begin(); j != types_.end(); ++j) {
        (*j).second->detach();
    }
}



void MarsLanguage::reset() {
    for (std::map<std::string, Type* >::iterator j = types_.begin(); j != types_.end(); ++j) {
        (*j).second->reset();
    }
}

eckit::Value MarsLanguage::jsonFile(const std::string& name) {
    // TODO: cache

    eckit::PathName path = std::string("~metkit/etc/" + name);

    std::ifstream in(path.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(path);
    }

    eckit::JSONParser parser(in);

    return parser.parse();
}

std::string MarsLanguage::bestMatch(const std::string& name,
                                    const std::vector<std::string>& values,
                                    bool fail,
                                    const std::map<std::string, std::string>& aliases) {

    size_t score = 0;
    std::vector<std::string> best;

    for (size_t i = 0; i < values.size(); ++i) {
        const std::string& value = values[i];

        size_t len = std::min(name.length(), value.length());
        size_t s = 0;

        for (size_t j = 0; j < len; ++j) {
            if (::tolower(name[j]) == ::tolower(value[j])) {
                s++;
            }
            else {
                break;
            }
        }

        if (s == value.length() && s == name.length()) {
            if (aliases.find(value) != aliases.end()) {
                return aliases.find(value)->second;
            }
            return value;
        }

        if (s > 0 && s >= score) {
            if (s > score) {
                best.clear();
            }
            best.push_back(value);
            score = s;
        }
    }

    size_t max = 3;
    if (best.size() > 0 && score < max) {
        std::cerr << "Matching '"
                  << name
                  << "' with "
                  << best
                  << " "
                  << "Please give at least " << max << " first letters"
                  << std::endl;
    }


    if (best.size() == 1) {
        if (aliases.find(best[0]) != aliases.end()) {
            return aliases.find(best[0])->second;
        }
        return best[0];
    }

    if (best.empty()) {

        if (!fail) {
            static std::string empty;
            return empty;
        }

        std::ostringstream oss;
        oss << "Cannot match '" << name << "' in " << values;
        throw eckit::UserError(oss.str());
    }

    std::set<std::string> names;
    for (std::vector<std::string>::const_iterator j = best.begin(); j != best.end(); ++j) {
        std::map<std::string, std::string>::const_iterator k = aliases.find(*j);
        if (k == aliases.end()) {
            names.insert(*j);
        }
        else {
            names.insert((*k).second);
        }
    }

    if (names.size() == 1) {
        return best[0];
    }

    std::ostringstream oss;
    oss << "Ambiguous value '" << name << "' could be " << best;
    throw eckit::UserError(oss.str());
}

std::string MarsLanguage::expandVerb(const std::string& verb) {
    pthread_once(&once, init);
    // std::map<std::string, std::string>::iterator c = cache_.find(verb);
    // if(c != cache_.end()) {
    //     return (*c).second;
    // }

    // return cache_[verb] = bestMatch(verb, verbs_, true);
    return bestMatch(verb, verbs_, true);
}

Type* MarsLanguage::type(const std::string& name) const {
    std::map<std::string, Type* >::const_iterator k = types_.find(name);
    if (k == types_.end()) {
        throw eckit::SeriousBug("Cannot find a type for '" + name + "'");
    }
    return (*k).second;
}


MarsRequest MarsLanguage::expand(const MarsRequest& r, bool inherit)  {
    // std::cout << r << std::endl;

    MarsRequest result(verb_);

    try {

        std::vector<std::string> params = r.params();
        std::set<std::string> seen;

        for (std::vector<std::string>::iterator j = params.begin(); j != params.end(); ++j) {
            std::string p;


            std::map<std::string, std::string>::iterator c = cache_.find(*j);
            if (c != cache_.end()) {
                p = (*c).second;
            } else {
                p =  cache_[*j] = bestMatch(*j, keywords_, true, aliases_);
            }

            // if (seen.find(p) != seen.end()) {
            //     std::cout << "Duplicate " << p << " " << *j << std::endl;
            //     std::cout << r << std::endl;
            //     if (result.countValues(p)) {
            //         std::cout << result.values(p) << std::endl;
            //     }
            //     else {
            //         std::cout << "off" << std::endl;
            //     }
            //     std::cout << r.values(*j) << std::endl;
            // }

            // seen.insert(p);

            std::vector<std::string> values = r.values(*j);

            if (values.size() == 1) {
                const std::string& s = values[0];
                if (s == "off" || s == "OFF") {
                    result.unsetValues(p);
                    type(p)->clearDefaults();
                    continue;
                }
            }

            type(p)->expand(values);
            result.setValuesTyped(type(p), values);

            // result.setValues(p, values);

        }



        if (inherit) {
            for (std::map<std::string, Type*>::iterator k = types_.begin(); k != types_.end(); ++k) {
                const std::string& name = (*k).first;
                if (result.countValues(name) == 0) {
                    (*k).second->setDefaults(result);
                }
            }

            result.getParams(params);
            for (std::vector<std::string>::const_iterator k = params.begin(); k != params.end(); ++k) {
                type(*k)->setDefaults(result.values(*k));
            }
        }

        result.getParams(params);

        for (std::vector<std::string>::const_iterator k = params.begin(); k != params.end(); ++k) {
            type(*k)->pass2(result);
        }

        for (std::vector<std::string>::const_iterator k = params.begin(); k != params.end(); ++k) {
            type(*k)->finalise(result);
        }

    }
    catch (Exception& e) {
        std::ostringstream oss;
        oss << e.what() << " request=" << r << ", expanded=" << result;
        throw eckit::UserError(oss.str());
    }
    return result;

}


const std::string& MarsLanguage::verb() const {
    return verb_;
}


void MarsLanguage::flatten(const MarsRequest& request,
                           const std::vector<std::string>& params,
                           size_t i,
                           MarsRequest& result,
                           FlattenCallback& callback) {

    if (i == params.size()) {
        callback(result);
        return;
    }

    const std::string& param = params[i];

    Type* t = type(param);
    if (!t->flatten()) {
        flatten(request, params, i + 1, result, callback);
        return;
    }

    const std::vector<std::string>& values = t->flattenValues(request);

    for (std::vector<std::string>::const_iterator j = values.begin(); j != values.end(); ++j) {
        result.setValue(param, *j);
        flatten(request, params, i + 1, result, callback);
    }

}

void MarsLanguage::flatten(const MarsRequest & request,
                           FlattenCallback & callback) {
    std::vector<std::string> params;
    request.getParams(params);

    MarsRequest result(request);
    flatten(request, params, 0, result, callback);

}

//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
