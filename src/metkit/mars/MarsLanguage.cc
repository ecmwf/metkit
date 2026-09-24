/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/MarsLanguage.h"

#include <algorithm>
#include <fstream>
#include <mutex>
#include <optional>

#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"

#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypesFactory.h"

//----------------------------------------------------------------------------------------------------------------------


static pthread_once_t once = PTHREAD_ONCE_INIT;

static eckit::Value languages_;
static std::vector<eckit::Value> modifiers_{};

namespace metkit::mars {

void initLanguage() {
    MarsLanguage::init();
}

//----------------------------------------------------------------------------------------------------------------------

ExpansionContext::ExpansionContext(const MarsRequest& request) {
    for (const auto& param : request.parameters()) {
        values_[MarsLanguage::keyword(param.name())] = param.values();
    }
}

ExpansionContext& ExpansionContext::operator=(ExpansionContext&& other) {
    values_ = std::move(other.values_);
    return *this;
}

bool ExpansionContext::has(Keyword key) const {
    return values_.find(key) != values_.end();
}

const std::vector<std::string>& ExpansionContext::values(Keyword key) const {
    static const std::vector<std::string> empty;
    auto it = values_.find(key);
    if (it != values_.end()) {
        return it->second;
    }
    return empty;
}

void ExpansionContext::unset(Keyword key) {
    values_.erase(key);
}

//----------------------------------------------------------------------------------------------------------------------

Dictionary<Verb> MarsLanguage::verbs_{};
Dictionary<Keyword> MarsLanguage::keywords_{};

void MarsLanguage::init() {
    languages_ = eckit::YAMLParser::decodeFile(metkit::LibMetkit::languageYamlFile());
    for (const auto& file : metkit::LibMetkit::modifiersYamlFiles()) {
        modifiers_.push_back(eckit::YAMLParser::decodeFile(file));
    }

    const eckit::Value verbs = languages_.keys();
    for (size_t i = 0; i < verbs.size(); ++i) {
        std::string verb  = verbs[i];
        auto k            = verbs_.add(verb);
        eckit::Value lang = languages_[verb];
        if (lang.contains("_aliases")) {
            eckit::Value aliases = lang["_aliases"];
            ASSERT(aliases.isList());
            for (size_t j = 0; j < aliases.size(); ++j) {
                std::string alias = aliases[j];
                verbs_.alias(alias, k);
            }
        }
    }
    for (const std::string& a : hypercube::AxisOrder::instance().axes()) {
        keywords_.add(a);
    }
}

void MarsLanguage::parseModifier(ModifierType typ, std::shared_ptr<Context> ctx, size_t maxIndex,
                                 const eckit::Value& mod) {
    eckit::Value keys;
    if (typ == ModifierType::UNSET) {
        ASSERT(mod.isList());
        keys = mod;
    }
    else {
        ASSERT(mod.isMap());
        keys = mod.keys();
    }
    for (size_t j = 0; j < keys.size(); ++j) {
        std::string keyStr = keys[j];
        Keyword key        = keywords_.exist(keyStr);
        if (key) {  // a keyword in the modifier might apply only to a language for another verb
            keyStr = keywords_.name(key);

            auto it = types_.find(key);
            if (it != types_.end()) {
                ASSERT(it->second->category() != Category::Data ||
                       maxIndex <= metkit::hypercube::AxisOrder::instance().index(keywords_.name(key)));

                if (typ == ModifierType::UNSET) {
                    it->second->unset(ctx);
                }
                else {
                    eckit::Value vv = mod[keyStr];
                    std::vector<std::string> vals;
                    if (vv.isList()) {
                        for (size_t k = 0; k < vv.size(); ++k) {
                            vals.push_back(vv[k]);
                        }
                    }
                    else {
                        vals.push_back(vv);
                    }

                    if (typ == ModifierType::DEFAULT) {
                        it->second->defaults(ctx, vals);
                    }
                    else if (typ == ModifierType::SET) {
                        it->second->set(ctx, vals);
                    }
                }
            }
        }
    }
}

MarsLanguage::MarsLanguage(const std::string& verb) {
    pthread_once(&once, initLanguage);
    verb_ = verbs_.keyword(verb);
    parse(verb_);
}

MarsLanguage::MarsLanguage(Verb verb) : verb_(verb) {
    pthread_once(&once, initLanguage);
    parse(verb_);
}

void MarsLanguage::parse(Verb verb) {

    eckit::Value lang    = languages_[verbs_.name(verb)];
    eckit::Value params  = lang.keys();
    eckit::Value options = lang["_options"];

    for (size_t i = 0; i < params.size(); ++i) {
        std::string keywordStr = params[i];
        if (keywordStr[0] == '_') {
            continue;
        }

        Keyword keyword = keywords_.add(keywordStr);
        keywordList_.push_back(keywordStr);

        ASSERT(types_.find(keyword) == types_.end());

        eckit::Value settings = lang[keywordStr];

        if (options.contains(keywordStr)) {
            eckit::ValueMap m = options[keywordStr];
            for (auto j = m.begin(); j != m.end(); ++j) {
                settings[(*j).first] = (*j).second;
            }
        }

        auto [it, success] = types_.emplace(keyword, TypesFactory::build(keyword, settings));
        ASSERT(success);
        it->second->attach();

        std::optional<eckit::Value> aliases;
        if (settings.contains("aliases")) {
            aliases = settings["aliases"];
        }
        if (aliases) {
            for (size_t j = 0; j < aliases->size(); ++j) {
                aliases_[(*aliases)[j]] = keyword;
                keywords_.alias((*aliases)[j], keyword);
            }
        }
    }

    // load modifiers and associate to types
    for (const auto& modifierFile : modifiers_) {
        ASSERT(modifierFile.isList());
        for (size_t i = 0; i < modifierFile.size(); ++i) {
            eckit::Value mod = modifierFile[i];
            ASSERT(mod.isMap());
            ASSERT(mod.contains("context"));

            std::shared_ptr<Context> ctx = Context::parseContext(mod["context"]);
            size_t maxIndex              = ctx->maxAxisIndex();
            if (mod.contains("defaults")) {
                parseModifier(ModifierType::DEFAULT, ctx, maxIndex, mod["defaults"]);
            }
            if (mod.contains("set")) {
                parseModifier(ModifierType::SET, ctx, maxIndex, mod["set"]);
            }
            if (mod.contains("unset")) {
                parseModifier(ModifierType::UNSET, ctx, maxIndex, mod["unset"]);
            }
        }
    }

    if (lang.contains("_clear_defaults")) {
        const auto& keywords = lang["_clear_defaults"];
        for (auto i = 0; i < keywords.size(); ++i) {
            Keyword key = keywords_.keyword(keywords[i]);
            if (auto iter = types_.find(key); iter != types_.end()) {
                iter->second->clearDefaults();
            }
        }
    }

    for (const std::string& a : hypercube::AxisOrder::instance().axes()) {
        Type* t     = nullptr;
        Keyword key = keywords_.exist(a);
        if (key) {
            auto it = types_.find(key);
            if (it != types_.end()) {
                t = it->second;
            }
            typesByAxisOrder_.emplace_back(key, t);
        }
    }
    for (auto& [k, t] : types_) {
        if (t->category() != Category::Data) {
            typesByAxisOrder_.emplace_back(k, t);
        }
    }
}

Category MarsLanguage::group(Keyword keyword) const {
    auto it = types_.find(keyword);
    if (it != types_.end()) {
        return it->second->category();
    }
    throw eckit::UserError("Cannot find keyword: " + std::to_string(keyword));
}

bool MarsLanguage::isData(Keyword k) const {
    return group(k) == Category::Data;
}
bool MarsLanguage::isDerived(Keyword k) const {
    return group(k) == Category::Derived;
}
bool MarsLanguage::isPostProc(Keyword k) const {
    return group(k) == Category::PostProc;
}
bool MarsLanguage::isSink(Keyword k) const {
    return group(k) == Category::Sink;
}

bool MarsLanguage::isData(const std::string& k) const {
    return group(keywords_.keyword(k)) == Category::Data;
}
bool MarsLanguage::isDerived(const std::string& k) const {
    return group(keywords_.keyword(k)) == Category::Derived;
}
bool MarsLanguage::isPostProc(const std::string& k) const {
    return group(keywords_.keyword(k)) == Category::PostProc;
}
bool MarsLanguage::isSink(const std::string& k) const {
    return group(keywords_.keyword(k)) == Category::Sink;
}

MarsLanguage::~MarsLanguage() {
    for (auto& [k, t] : types_) {
        t->detach();
    }
}

const MarsLanguage& MarsLanguage::get(Verb verb) {
    static std::mutex mutex;
    static std::map<Verb, MarsLanguage*> instances;

    std::lock_guard lock(mutex);
    auto it = instances.find(verb);
    if (it != instances.end()) {
        return *(it->second);
    }

    auto [newIt, inserted] = instances.emplace(verb, new MarsLanguage(verb));
    ASSERT(inserted);
    return *(newIt->second);
}

const MarsLanguage& MarsLanguage::get(const std::string& verb) {
    pthread_once(&once, initLanguage);
    return get(verbs_.keyword(verb));
}

eckit::Value MarsLanguage::jsonFile(const std::string& name) {
    // TODO: cache

    eckit::PathName path = metkit::LibMetkit::configFile(name);

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

std::string MarsLanguage::bestMatch(const std::string& name, const std::vector<std::string>& values, bool fail,
                                    bool quiet, bool fullMatch, const std::map<std::string, std::string>& aliases) {
    size_t score = (fullMatch ? name.length() : 1);
    std::vector<std::string> best;

    for (size_t i = 0; i < values.size(); ++i) {
        const std::string& value = values[i];

        size_t len = std::min(name.length(), value.length());
        size_t s   = 0;

        // Prefix comparison, break if there is a difference
        for (size_t j = 0; j < len; ++j) {
            if (::tolower(name[j]) == ::tolower(value[j])) {
                s++;
            }
            else {
                break;
            }
        }

        // if value and name are the same (in lowercase), look up aliases and return the alias
        // otherwise return the value (which is name)
        if (s == value.length() && s == name.length()) {
            if (aliases.find(value) != aliases.end()) {
                return aliases.find(value)->second;
            }
            return value;
        }

        // If fullmatch == true:
        // only option is s==score, otherwise the break above would have triggered
        // so best is only filled with exact matches
        // If fullmatch == false:
        // score keeps track of the best value found, if a better value (in terms of matching prefixes)
        // is found, clear the best vector and push the new best. If all matches are equally good, keep them in the best
        // vector
        if (s >= score) {
            if (s > score) {
                best.clear();
            }
            best.push_back(value);
            score = s;
        }
    }

    if (!quiet && best.size() > 0) {
        std::cerr << "Matching '" << name << "' with " << best << std::endl;
    }

    static bool strict = eckit::Resource<bool>("$METKIT_LANGUAGE_STRICT_MODE", true);
    if (best.size() == 1) {
        // If the best entry is a number or not name (why is this even needed)
        if (isnumeric(best[0]) && (best[0] != name)) {
            best.clear();
        }
        else {
            if (strict) {
                if (best[0] != name) {
                    std::ostringstream oss;
                    oss << "Cannot match [" << name << "] in " << values;
                    throw eckit::UserError(oss.str());
                }
            }

            if (aliases.find(best[0]) != aliases.end()) {
                return aliases.find(best[0])->second;
            }
            return best[0];
        }
    }

    // In case the match is empty, return or fail, depending on the fail flag
    static std::string empty;
    if (best.empty()) {
        if (!fail) {
            return empty;
        }

        std::ostringstream oss;
        oss << "Cannot match [" << name << "] in " << values;
        throw eckit::UserError(oss.str());
    }

    // Check the existing aliases for mappings of the matches to the canonical parameter name
    // A set is used to de-duplicate the findings
    std::set<std::string> names;
    for (std::vector<std::string>::const_iterator j = best.begin(); j != best.end(); ++j) {
        const auto k = aliases.find(*j);
        if (k == aliases.end()) {
            names.insert(*j);
        }
        else {
            names.insert((*k).second);
        }
    }

    // If there is only on match return the given one
    if (names.size() == 1) {
        if (aliases.find(best[0]) != aliases.end()) {
            return aliases.find(best[0])->second;
        }
        return best[0];
    }

    // Otherwise there is an ambiguity and we want to return or fail (depending on the fail flag)
    if (!fail) {
        return empty;
    }

    std::ostringstream oss;
    oss << "Ambiguous value '" << name << "' could be";

    for (std::vector<std::string>::const_iterator j = best.begin(); j != best.end(); ++j) {
        auto k = aliases.find(*j);
        if (k == aliases.end()) {
            oss << " '" << *j << "'";
        }
        else {
            oss << " '" << *j << "' (";
            oss << (*k).second;
            oss << ")";
        }
    }

    throw eckit::UserError(oss.str());
}

const std::string& MarsLanguage::expandVerb(const std::string& verb) {
    pthread_once(&once, initLanguage);
    return verbs_.name(verbs_.keyword(verb));
}

class TypeHidden : public Type {
    bool flatten() const override { return false; }
    void print(std::ostream& out) const override { out << "TypeHidden"; }
    bool expand(std::string&, const MarsRequest&) const override { return true; }

public:

    TypeHidden() : Type(MarsLanguage::addKeyword("hidden"), eckit::Value()) { attach(); }
};

const Type* MarsLanguage::type(Keyword name) const {
    auto it = types_.find(name);
    if (it == types_.end()) {
        throw eckit::SeriousBug("Cannot find a type for '" + keywords_.name(name) + "'");
    }
    return it->second;
}

const Type* MarsLanguage::type(const std::string& name) const {
    static TypeHidden hidden;
    Keyword key = keywords_.exist(name);
    if (key) {
        return type(keywords_.keyword(name));
    }
    if ((name)[0] == '_') {
        return &hidden;
    }

    throw eckit::SeriousBug("Cannot find a type for '" + name + "'");
}

MarsRequest MarsLanguage::expand(const MarsRequest& r, ExpansionContext& ctx, bool inherit, bool strict) const {
    MarsRequest result(verb_);

    try {
        std::vector<std::pair<Keyword, std::string>> sortedParams;
        std::map<Keyword, std::string> paramSet;
        std::vector<std::string> params;

        for (const auto& PP : r.params()) {
            std::string p = eckit::StringTools::lower(PP);
            Keyword key   = keywords_.exist(p);
            if (!key) {
                // fall back to fuzzy matching, governed by METKIT_LANGUAGE_STRICT_MODE
                p   = bestMatch(p, keywordList_, true, false, true, aliases_);
                key = keywords_.exist(p);
                ASSERT(key);
            }
            paramSet.emplace(key, PP);
        }
        {  // sort the parameters, following the AxisOrder
            for (const auto& a : metkit::hypercube::AxisOrder::instance().axes()) {
                Keyword k = keywords_.exist(a);
                if (k) {
                    auto it = paramSet.find(k);
                    if (it != paramSet.end()) {
                        sortedParams.emplace_back(k, it->second);
                        paramSet.erase(it);
                    }
                }
            }
            for (const auto& [k, PP] : paramSet) {
                sortedParams.emplace_back(k, PP);
            }
        }

        for (const auto& [k, PP] : sortedParams) {
            std::vector<std::string> values = r.values(PP);

            if (values.size() == 1) {
                const std::string& s = eckit::StringTools::lower(values[0]);
                if (s == "off") {
                    result.unsetValues(keywords_.name(k));
                    ctx.unset(k);
                    continue;
                }
                if (s == "all" && type(k)->multiple()) {
                    result.setValue(keywords_.name(k), "all");
                    continue;
                }
            }

            auto t = type(k);
            t->expand(values, result);
            result.setValuesTyped(t, values);
            t->check(values);
        }

        if (inherit) {
            for (const auto& [k, t] : typesByAxisOrder_) {
                if (t != nullptr && result.countValues(keywords_.name(k)) == 0) {
                    if (ctx.has(k)) {
                        result.setValuesTyped(t, ctx.values(k));
                    }
                    else {
                        t->setDefaults(result);
                    }
                }
            }
        }

        result.getParams(params);

        for (std::vector<std::string>::const_iterator k = params.begin(); k != params.end(); ++k) {
            type(*k)->pass2(result);
        }

        for (const auto& [k, t] : typesByAxisOrder_) {
            if (t != nullptr)
                t->finalise(result, strict);
        }
    }
    catch (std::exception& e) {
        std::ostringstream oss;
        oss << e.what() << " request=" << r << ", expanded=" << result;
        throw eckit::UserError(oss.str());
    }
    if (inherit) {
        ctx = ExpansionContext(result);
    }
    return result;
}

void MarsLanguage::flatten(const MarsRequest& request, const std::vector<std::string>& params, size_t i,
                           MarsRequest& result, FlattenCallback& callback) const {
    if (i == params.size()) {
        callback(result);
        return;
    }

    const std::string& param = params[i];

    const Type* t = type(param);
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

void MarsLanguage::flatten(const MarsRequest& request, FlattenCallback& callback) const {
    std::vector<std::string> params;
    request.getParams(params);

    MarsRequest result(request);
    flatten(request, params, 0, result, callback);
}

Verb MarsLanguage::verb(const std::string& name) {
    pthread_once(&once, initLanguage);
    return verbs_.keyword(name);
}
const std::string& MarsLanguage::name(Verb verb) {
    pthread_once(&once, initLanguage);
    return verbs_.name(verb);
}

Keyword MarsLanguage::addKeyword(const std::string& name) {
    return keywords_.add(name);
}
Keyword MarsLanguage::hasKeyword(const std::string& name) {
    return keywords_.exist(name);
}
Keyword MarsLanguage::keyword(const std::string& name) {
    return keywords_.keyword(name);
}
const std::string& MarsLanguage::name(Keyword keyword) {
    return keywords_.name(keyword);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
