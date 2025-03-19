/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <fstream>
#include <list>
#include <set>

#include "eckit/config/Resource.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/Types.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "metkit/config/LibMetkit.h"

#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypesFactory.h"

//----------------------------------------------------------------------------------------------------------------------


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Value languages_;
static std::vector<std::string> verbs_;

static void init() {
    languages_ = eckit::YAMLParser::decodeFile(metkit::mars::MarsLanguage::languageYamlFile());
    const eckit::Value verbs = languages_.keys();
    for (size_t i = 0; i < verbs.size(); ++i) {
        verbs_.push_back(verbs[i]);
    }
}


namespace metkit {
namespace mars {


//----------------------------------------------------------------------------------------------------------------------


MarsLanguage::MarsLanguage(const std::string& verb) : verb_(verb) {
    pthread_once(&once, init);

    eckit::Value lang   = languages_[verb];
    eckit::Value params = lang.keys();

    eckit::Value options  = lang["_options"];

    for (size_t i = 0; i < params.size(); ++i) {
        std::string keyword   = params[i];
        eckit::Value settings = lang[keyword];

        if (keyword[0] == '_') {
            continue;
        }

        ASSERT(types_.find(keyword) == types_.end());

        if (options.contains(keyword)) {
            eckit::ValueMap m = options[keyword];
            for (auto j = m.begin(); j != m.end(); ++j) {
                settings[(*j).first] = (*j).second;
            }
        }

        types_[keyword] = TypesFactory::build(keyword, settings);
        types_[keyword]->attach();
        keywords_.push_back(keyword);

        if (settings.contains("aliases")) {
            eckit::Value aliases = settings["aliases"];
            for (size_t j = 0; j < aliases.size(); ++j) {
                aliases_[aliases[j]] = keyword;
                keywords_.push_back(aliases[j]);
            }
        }
    }

    if (lang.contains("_clear_defaults")) {
        const auto& keywords = lang["_clear_defaults"];
        for (auto i = 0; i < keywords.size(); ++i) {
            if (auto iter = types_.find(keywords[i]); iter != types_.end()) { iter->second->clearDefaults(); }
        }
    }

    std::set<std::string> keywordsInAxis;
    for (const std::string& a : hypercube::AxisOrder::instance().axes()) {
        keywordsInAxis.insert(a);
        Type* t=nullptr;
        auto it = types_.find(a);
        if(it != types_.end()) {
            t = (*it).second;
        }
        typesByAxisOrder_.emplace_back(a, t);
    }
    for (const auto& [k,t] : types_) {
        if (keywordsInAxis.find(k) == keywordsInAxis.end()) { typesByAxisOrder_.emplace_back(k, t); }
    }
}

MarsLanguage::~MarsLanguage() {
    for (std::map<std::string, Type*>::iterator j = types_.begin(); j != types_.end(); ++j) {
        (*j).second->detach();
    }
}

eckit::PathName MarsLanguage::languageYamlFile() {
    return "~metkit/share/metkit/language.yaml";
}


void MarsLanguage::reset() {
    for (std::map<std::string, Type*>::iterator j = types_.begin(); j != types_.end(); ++j) {
        (*j).second->reset();
    }
}

eckit::Value MarsLanguage::jsonFile(const std::string& name) {
    // TODO: cache

    eckit::PathName path = std::string("~metkit/share/metkit/" + name);

    LOG_DEBUG_LIB(LibMetkit) << "MarsLanguage loading jsonFile " << path << std::endl;

    std::ifstream in(path.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(path);
    }

    eckit::YAMLParser parser(in);

    return parser.parse();
}

static bool isnumeric(const std::string& s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (!::isdigit(s[i])) {
            return false;
        }
    }

    return s.length() > 0;
}

std::string MarsLanguage::bestMatch(const MarsExpandContext& ctx, const std::string& name,
                                    const std::vector<std::string>& values, bool fail, bool quiet, bool fullMatch,
                                    const std::map<std::string, std::string>& aliases) {
    size_t score = (fullMatch ? name.length() : 1);
    std::vector<std::string> best;

    static bool strict = eckit::Resource<bool>("$METKIT_LANGUAGE_STRICT_MODE", false);


    for (size_t i = 0; i < values.size(); ++i) {
        const std::string& value = values[i];

        size_t len = std::min(name.length(), value.length());
        size_t s   = 0;

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

        if (s >= score) {
            if (s > score) {
                best.clear();
            }
            best.push_back(value);
            score = s;
        }
    }

    if (!quiet && best.size() > 0) {
        std::cerr << "Matching '" << name << "' with " << best << ctx << std::endl;
    }

    if (best.size() == 1) {
        if (isnumeric(best[0]) && (best[0] != name)) {
            best.clear();
        }
        else {
            if (strict) {
                if (best[0] != name) {
                    std::ostringstream oss;
                    oss << "Cannot match [" << name << "] in " << values << ctx;
                    throw eckit::UserError(oss.str());
                }
            }

            if (aliases.find(best[0]) != aliases.end()) {
                return aliases.find(best[0])->second;
            }
            return best[0];
        }
    }

    static std::string empty;
    if (best.empty()) {
        if (!fail) { return empty; }

        std::ostringstream oss;
        oss << "Cannot match [" << name << "] in " << values << ctx;
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
        if (aliases.find(best[0]) != aliases.end()) {
            return aliases.find(best[0])->second;
        }
        return best[0];
    }

    if (!fail) {
        return empty;
    }

    std::ostringstream oss;
    oss << "Ambiguous value '" << name << "' could be";

    for (std::vector<std::string>::const_iterator j = best.begin(); j != best.end(); ++j) {
        std::map<std::string, std::string>::const_iterator k = aliases.find(*j);
        if (k == aliases.end()) {
            oss << " '" << *j << "'";
        }
        else {
            oss << " '" << *j << "' (";
            oss << (*k).second;
            oss << ")";
        }
    }

    oss << ctx;

    throw eckit::UserError(oss.str());
}

std::string MarsLanguage::expandVerb(const MarsExpandContext& ctx, const std::string& verb) {
    pthread_once(&once, init);
    return bestMatch(ctx, verb, verbs_, true, true, false);
}

class TypeHidden : public Type {
    virtual bool flatten() const { return false; }
    virtual void print(std::ostream& out) const { out << "TypeHidden"; }
    virtual bool expand(const MarsExpandContext& ctx, std::string& value) const { return true; }

public:
    TypeHidden() : Type("hidden", eckit::Value()) { attach(); }
};


Type* MarsLanguage::type(const std::string& name) const {
    std::map<std::string, Type*>::const_iterator k = types_.find(name);
    if (k == types_.end()) {
        if (name[0] == '_') {
            static TypeHidden hidden;
            return &hidden;
        }

        throw eckit::SeriousBug("Cannot find a type for '" + name + "'");
    }
    return (*k).second;
}


MarsRequest MarsLanguage::expand(const MarsExpandContext& ctx, const MarsRequest& r, bool inherit, bool strict) {
    MarsRequest result(verb_);

    try {
        std::vector<std::string> params = r.params();
        std::set<std::string> seen;

        for (std::vector<std::string>::iterator j = params.begin(); j != params.end(); ++j) {
            std::string p;


            std::map<std::string, std::string>::iterator c = cache_.find(*j);
            if (c != cache_.end()) {
                p = (*c).second;
            }
            else {
                p = cache_[*j] = bestMatch(ctx, *j, keywords_, true, false, false, aliases_);
            }

            std::vector<std::string> values = r.values(*j);

            if (values.size() == 1) {
                const std::string& s = values[0];
                if (s == "off" || s == "OFF") {
                    result.unsetValues(p);
                    type(p)->clearDefaults();
                    continue;
                }
            }

            type(p)->expand(ctx, values);
            result.setValuesTyped(type(p), values);
            type(p)->check(ctx, values);
        }


        if (inherit) {
            for (const auto& [k,t] : typesByAxisOrder_) {
                if (t != nullptr && result.countValues(k) == 0) {
                    t->setDefaults(result);
                }
            }

            result.getParams(params);
            for (std::vector<std::string>::const_iterator k = params.begin(); k != params.end();
                 ++k) {
                type(*k)->setInheritance(result.values(*k));
            }
        }

        result.getParams(params);

        for (std::vector<std::string>::const_iterator k = params.begin(); k != params.end(); ++k) {
            type(*k)->pass2(ctx, result);
        }

        for (std::vector<std::string>::const_iterator k = params.begin(); k != params.end(); ++k) {
            type(*k)->finalise(ctx, result, strict);
        }
    }
    catch (std::exception& e) {
        std::ostringstream oss;
        oss << e.what() << " request=" << r << ", expanded=" << result;
        throw eckit::UserError(oss.str());
    }
    return result;
}


const std::string& MarsLanguage::verb() const {
    return verb_;
}


void MarsLanguage::flatten(const MarsRequest& request, const std::vector<std::string>& params,
                           size_t i, MarsRequest& result, FlattenCallback& callback) {
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

void MarsLanguage::flatten(const MarsExpandContext&, const MarsRequest& request,
                           FlattenCallback& callback) {
    std::vector<std::string> params;
    request.getParams(params);

    MarsRequest result(request);
    flatten(request, params, 0, result, callback);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit
