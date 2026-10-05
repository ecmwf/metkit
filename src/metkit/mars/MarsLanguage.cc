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
#include <shared_mutex>

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
        Keyword key = param.id();
        if (key) {  // a parameter that is not a registered keyword cannot be inherited by any language
            values_[key] = param.values();
        }
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

// static members of MarsLanguage
std::unique_ptr<Dictionary<Verb>> MarsLanguage::verbs_{};
std::unique_ptr<Dictionary<Keyword>> MarsLanguage::keywords_{};
Keyword MarsLanguage::maxDataKeyword_ = 0;
std::shared_mutex MarsLanguage::contextsMutex_;
std::map<Keyword, size_t> MarsLanguage::langOffsets_{};
std::set<Context*, ContextPtrComparator> MarsLanguage::contextsSet_{};
std::vector<std::unique_ptr<Context>> MarsLanguage::contexts_{};
MemFile MarsLanguage::langFile_;

void MarsLanguage::loadYaml() {
    static std::once_flag loaded;
    std::call_once(loaded, [] {
        languages_ = eckit::YAMLParser::decodeFile(metkit::LibMetkit::languageYamlFile());
        for (const auto& file : metkit::LibMetkit::modifiersYamlFiles()) {
            modifiers_.push_back(eckit::YAMLParser::decodeFile(file));
        }
    });
}

bool MarsLanguage::loadBinary() {
    static bool metkitForceBinfileCreation = eckit::Resource<bool>("$METKIT_FORCE_BINFILE_CREATION", false);

    if (metkitForceBinfileCreation) {
        return false;
    }

    eckit::PathName languageBinFile = LibMetkit::languageBinaryFile();
    if (!languageBinFile.exists()) {
        eckit::Log::info() << "Language binary file '" << languageBinFile.asString()
                           << "' missing - using slow config file parsing" << std::endl;
        return false;
    }

    try {
        langFile_ = MemFile{languageBinFile.localPath()};

        std::string header{langFile_.readString(4)};
        uint16_t version = langFile_.read16();

        LOG_DEBUG_LIB(LibMetkit) << "Reading language binary file header: " << header << " version: " << version
                                 << std::endl;

        if ("LANG" == header && version == LibMetkit::binaryFilesVersion()) {

            readDictionaries(langFile_);

            uint8_t numVerbs = langFile_.read8();
            for (uint8_t u = 0; u < numVerbs; ++u) {
                Verb vv         = langFile_.read8();
                uint32_t offset = langFile_.read32();
                langOffsets_.emplace(vv, offset);
            }
            ASSERT(langOffsets_.size() == numVerbs);

            // the contexts follow the verb offsets table and are shared by all the languages
            readContexts(langFile_);

            return true;
        }

        eckit::Log::warning() << "Incompatible language binary file '" << languageBinFile.asString()
                              << "' - expected header LANG version " << LibMetkit::binaryFilesVersion() << ", found "
                              << header << " version " << version << " - using slow config file parsing" << std::endl;
    }
    catch (const std::exception& e) {
        eckit::Log::warning() << "Error reading language binary file '" << languageBinFile.asString()
                              << "': " << e.what() << " - using slow config file parsing" << std::endl;
    }

    // do not leave a half-loaded state behind: the YAML parsing below starts from scratch
    verbs_.reset();
    keywords_.reset();
    maxDataKeyword_ = 0;
    langOffsets_.clear();
    {
        std::unique_lock lock(contextsMutex_);
        contextsSet_.clear();
        contexts_.clear();
    }
    langFile_ = MemFile{};  // releases the mapping
    return false;
}

void MarsLanguage::init() {
    if (loadBinary()) {
        return;
    }

    loadYaml();

    verbs_                   = std::make_unique<Dictionary<Verb>>("verb");
    const eckit::Value verbs = languages_.keys();
    for (size_t i = 0; i < verbs.size(); ++i) {
        std::string verb  = verbs[i];
        auto k            = verbs_->add(verb);
        eckit::Value lang = languages_[verb];
        if (lang.contains("_aliases")) {
            eckit::Value aliases = lang["_aliases"];
            ASSERT(aliases.isList());
            for (size_t j = 0; j < aliases.size(); ++j) {
                std::string alias = aliases[j];
                verbs_->alias(alias, k);
            }
        }
    }
    keywords_ = std::make_unique<Dictionary<Keyword>>("keyword");
    keywords_->add(std::string{"_verb"});
    for (const std::string& a : hypercube::AxisOrder::instance().axes()) {
        keywords_->add(a);
    }
    maxDataKeyword_ = keywords_->size();
    keywords_->add(std::string{"day"});
    keywords_->add(std::string{"output"});
    keywords_->add(std::string{"pseudodate"});

    verbs_->add(std::string{"verb"});
    verbs_->add(std::string{"environ"});

    auto emptyValue = eckit::Value{};
    addContext(emptyValue);
}

void MarsLanguage::writeDictionaries(std::ofstream& file) {
    verbs_->serialize(file);
    keywords_->serialize(file);
    write16(file, maxDataKeyword_);
}
void MarsLanguage::readDictionaries(MemFile& file) {
    verbs_          = std::make_unique<Dictionary<Verb>>(file, "verb");
    keywords_       = std::make_unique<Dictionary<Keyword>>(file, "keyword");
    maxDataKeyword_ = file.read16();
}

void MarsLanguage::writeContexts(std::ofstream& file) {
    write16(file, contexts_.size());
    for (const auto& context : contexts_) {
        context->write(file);
    }
}
void MarsLanguage::readContexts(MemFile& file) {
    size_t size = file.read16();
    for (size_t i = 0; i < size; ++i) {
        auto context = std::make_unique<Context>(i, file);
        contextsSet_.insert(context.get());
        contexts_.push_back(std::move(context));
    }
}

void MarsLanguage::write(std::ofstream& file) const {
    // write one language

    // write the verb identifier at the beginning of the language block
    write8(file, verb_);
    // write the number of types
    write16(file, types_.size());
    for (const auto& [keyword, type] : types_) {
        // delegate writing of the type to the type itself
        type->write(file);
    }

    // data structures supporting bestMatch (counts are 16 bits: the lists can exceed 255 entries)
    write16(file, keywordList_.size());
    for (const auto& keyword : keywordList_) {
        writeString(file, keyword);
    }
    write16(file, aliases_.size());
    for (const auto& [alias, keyword] : aliases_) {
        writeString(file, alias);
        writeString(file, keyword);
    }
}

void MarsLanguage::parseModifier(ModifierType typ, const Context& ctx, size_t maxIndex, const eckit::Value& mod) {
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
        Keyword key        = keywords_->exist(keyStr);
        if (key) {  // a keyword in the modifier might apply only to a language for another verb
            keyStr = keywords_->name(key);

            auto it = types_.find(key);
            if (it != types_.end()) {
                ASSERT(it->second->category() != Category::Data || maxIndex <= key);

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

const Context& MarsLanguage::context(size_t ctxId) {
    std::shared_lock lock(contextsMutex_);
    ASSERT(ctxId < contexts_.size());
    const auto& ctx = contexts_[ctxId];
    ASSERT(ctx != nullptr);
    return *ctx;
}

const Context& MarsLanguage::addContext(eckit::Value& val) {
    std::unique_lock lock(contextsMutex_);
    auto ctx = std::make_unique<Context>(contexts_.size(), val);
    auto it  = contextsSet_.find(ctx.get());
    if (it != contextsSet_.end()) {
        return *(*it);
    }
    contexts_.push_back(std::move(ctx));
    contextsSet_.insert(contexts_.back().get());
    return *(contexts_.back());
}


MarsLanguage::MarsLanguage(const std::string& verb) {
    pthread_once(&once, initLanguage);
    verb_ = verbs_->keyword(eckit::StringTools::lower(verb));
    parse(verb_);
}

MarsLanguage::MarsLanguage(Verb verb) : verb_(verb) {
    pthread_once(&once, initLanguage);
    parse(verb_);
}

MarsLanguage::MarsLanguage(Verb verb, MemFile& file) {
    pthread_once(&once, initLanguage);

    auto offset = langOffsets_.find(verb);
    ASSERT(offset != langOffsets_.end());
    file.seek(offset->second);
    verb_ = file.read8();
    ASSERT(verb_ == verb);

    uint16_t numTypes = file.read16();
    for (uint16_t i = 0; i < numTypes; ++i) {
        std::string typeName{file.readString()};
        Keyword keyword = file.read16();
        auto type       = TypesFactory::build(typeName, keyword, file);
        types_.emplace(keyword, type);
    }

    uint16_t numKeywords = file.read16();
    keywordList_.reserve(numKeywords);
    for (uint16_t i = 0; i < numKeywords; ++i) {
        keywordList_.emplace_back(file.readString());
    }

    uint16_t numAliases = file.read16();
    for (uint16_t i = 0; i < numAliases; ++i) {
        std::string alias{file.readString()};
        std::string keyword{file.readString()};
        aliases_.emplace(std::move(alias), std::move(keyword));
    }
}

void MarsLanguage::parse(Verb verb) {

    loadYaml();  // not loaded yet if the binary file is used for the other verbs

    eckit::Value lang    = languages_[verbs_->name(verb)];
    eckit::Value params  = lang.keys();
    eckit::Value options = lang["_options"];

    for (size_t i = 0; i < params.size(); ++i) {
        std::string keywordStr = params[i];
        if (keywordStr[0] == '_') {
            continue;
        }

        Keyword keyword = keywords_->add(keywordStr);
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

        std::optional<eckit::Value> aliases;
        if (settings.contains("aliases")) {
            aliases = settings["aliases"];
        }
        if (aliases) {
            for (size_t j = 0; j < aliases->size(); ++j) {
                std::string alias = (*aliases)[j];
                aliases_[alias]   = keywordStr;  // alias name -> keyword name
                keywordList_.push_back(alias);   // aliases take part in the fuzzy matching, as before
                keywords_->alias(alias, keyword);
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

            const Context& ctx = addContext(mod["context"]);
            size_t maxIndex    = ctx.maxAxisIndex();
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
            Keyword key = keywords_->exist(keywords[i]);
            if (auto iter = types_.find(key); key && iter != types_.end()) {
                iter->second->clearDefaults();
            }
        }
    }
}

Category MarsLanguage::category(Keyword keyword) const {
    auto it = types_.find(keyword);
    if (it != types_.end()) {
        return it->second->category();
    }
    throw eckit::UserError("Cannot find keyword: " + keywords_->name(keyword));
}

bool MarsLanguage::isData(Keyword k) const {
    return category(k) == Category::Data;
}
bool MarsLanguage::isDerived(Keyword k) const {
    return category(k) == Category::Derived;
}
bool MarsLanguage::isPostProc(Keyword k) const {
    return category(k) == Category::PostProc;
}
bool MarsLanguage::isSink(Keyword k) const {
    return category(k) == Category::Sink;
}

bool MarsLanguage::isData(const std::string& k) const {
    return category(keywords_->keyword(k)) == Category::Data;
}
bool MarsLanguage::isDerived(const std::string& k) const {
    return category(keywords_->keyword(k)) == Category::Derived;
}
bool MarsLanguage::isPostProc(const std::string& k) const {
    return category(keywords_->keyword(k)) == Category::PostProc;
}
bool MarsLanguage::isSink(const std::string& k) const {
    return category(keywords_->keyword(k)) == Category::Sink;
}

const MarsLanguage& MarsLanguage::get(Verb verb) {
    static std::mutex mutex;
    static std::map<Verb, MarsLanguage*> instances;

    static bool metkitForceBinfileCreation = eckit::Resource<bool>("$METKIT_FORCE_BINFILE_CREATION", false);

    std::lock_guard lock(mutex);
    auto it = instances.find(verb);
    if (it != instances.end()) {
        return *(it->second);
    }

    // load / parse the language for the given verb
    if (!metkitForceBinfileCreation && !langOffsets_.empty()) {
        auto it = langOffsets_.find(verb);
        if (it != langOffsets_.end()) {

            auto [newIt, inserted] = instances.emplace(verb, new MarsLanguage(verb, langFile_));
            ASSERT(inserted);
            return *(newIt->second);
        }
    }
    auto [newIt, inserted] = instances.emplace(verb, new MarsLanguage(verb));
    ASSERT(inserted);
    return *(newIt->second);
}

const MarsLanguage& MarsLanguage::get(const std::string& verb) {
    pthread_once(&once, initLanguage);
    return get(verbs_->keyword(eckit::StringTools::lower(verb)));
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
    return verbs_->name(verbs_->keyword(eckit::StringTools::lower(verb)));
}

class TypeHidden : public Type {
    bool flatten() const override { return false; }
    void print(std::ostream& out) const override { out << "TypeHidden"; }
    bool expand(std::string&, const MarsRequest&) const override { return true; }

public:

    TypeHidden() : Type("hidden", MarsLanguage::addKeyword("hidden"), eckit::Value()) {}
    ~TypeHidden() override = default;
};

const Type* MarsLanguage::type(Keyword key) const {
    auto it = types_.find(key);
    if (it == types_.end()) {
        throw eckit::UserError("Keyword '" + keywords_->name(key) + "' is not valid for verb '" + verbs_->name(verb_) +
                               "'");
    }
    return it->second.get();
}

const Type* MarsLanguage::type(const std::string& name) const {
    Keyword key = keywords_->exist(name);
    if (key && types_.find(key) != types_.end()) {
        return type(key);
    }
    if (!name.empty() && name[0] == '_') {  // internal keywords (e.g. _verb) are known, but have no type in a language
        static const TypeHidden hidden{};
        return &hidden;
    }

    throw eckit::UserError("Cannot find a type for '" + name + "'");
}

MarsRequest MarsLanguage::expand(const MarsRequest& r, ExpansionContext& ctx, bool inherit, bool strict) const {
    MarsRequest result(verb_);

    try {
        // Resolve the keywords of the request in this language. They are processed sorted by keyword (axes first,
        // in axis order), so the order of the parameters in the request does not matter.
        std::map<Keyword, const Parameter*> paramSet;

        for (const auto& param : r.parameters()) {
            // a typed parameter that is already a keyword of this language does not need to be looked up
            Keyword key = param.typed() ? param.id() : 0;
            if (!key || types_.find(key) == types_.end()) {
                std::string p = eckit::StringTools::lower(param.name());
                key           = keywords_->exist(p);
                if (!key || types_.find(key) == types_.end()) {
                    // not a keyword of this language (the dictionary is shared by all the verbs):
                    // fall back to fuzzy matching, governed by METKIT_LANGUAGE_STRICT_MODE
                    p   = bestMatch(p, keywordList_, true, false, true, aliases_);
                    key = keywords_->exist(p);
                }
            }
            paramSet.emplace(key, &param);
        }

        for (const auto& [k, param] : paramSet) {
            std::vector<std::string> values = param->values();  // copy the values to expand in place

            if (values.size() == 1) {
                const std::string s = eckit::StringTools::lower(values[0]);
                if (s == "off") {
                    result.erase(k);
                    ctx.unset(k);
                    continue;
                }
                if (s == "all" && type(k)->multiple()) {
                    result.setValuesTyped(type(k), std::vector<std::string>{"all"});
                    continue;
                }
            }

            auto t = type(k);
            t->expand(values, result);
            result.setValuesTyped(t, values);
            t->check(values);
        }

        if (inherit) {
            for (const auto& [k, t] : types_) {
                if (isOrdered(k, *t) && result.countValues(k) == 0) {
                    if (ctx.has(k)) {
                        result.setValuesTyped(t, ctx.values(k));
                    }
                    else {
                        t->setDefaults(result);
                    }
                }
            }
        }

        // pass2 modifies the request: go through a snapshot of the types, not through the parameters being modified
        std::vector<std::shared_ptr<const Type>> expanded;
        expanded.reserve(result.parameters().size());
        for (const auto& param : result.parameters()) {
            expanded.push_back(param.typePtr());
        }
        for (const auto& t : expanded) {
            ASSERT(t);
            t->pass2(result);
        }

        for (const auto& [k, t] : types_) {
            if (isOrdered(k, *t)) {
                t->finalise(result, strict);
            }
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

void MarsLanguage::flatten(const MarsRequest& request, const std::vector<Keyword>& params, size_t i,
                           MarsRequest& result, FlattenCallback& callback) const {
    if (i == params.size()) {
        callback(result);
        return;
    }

    Keyword param = params[i];

    // internal keywords (e.g. _verb) are registered, but have no type in the language: look them up by name
    const Type* t = types_.find(param) != types_.end() ? type(param) : type(name(param));
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
    MarsRequest result(request);
    std::vector<Keyword> params;
    for (const auto& p : request.parameters()) {
        Keyword key = p.id();
        if (!key) {
            throw eckit::UserError("Cannot flatten a request with an unknown keyword '" + p.name() + "'");
        }
        params.push_back(key);
    }
    flatten(request, params, 0, result, callback);
}

Verb MarsLanguage::verb(const std::string& name) {
    pthread_once(&once, initLanguage);
    return verbs_->keyword(eckit::StringTools::lower(name));
}
const std::string& MarsLanguage::name(Verb verb) {
    pthread_once(&once, initLanguage);
    return verbs_->name(verb);
}

Keyword MarsLanguage::addKeyword(const std::string& name) {
    pthread_once(&once, initLanguage);
    return keywords_->add(eckit::StringTools::lower(name));
}
Keyword MarsLanguage::hasKeyword(const std::string& name) {
    pthread_once(&once, initLanguage);
    return keywords_->exist(eckit::StringTools::lower(name));
}
Keyword MarsLanguage::keyword(const std::string& name) {
    pthread_once(&once, initLanguage);
    return keywords_->keyword(eckit::StringTools::lower(name));
}
const std::string& MarsLanguage::name(Keyword keyword) {
    pthread_once(&once, initLanguage);
    return keywords_->name(keyword);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
