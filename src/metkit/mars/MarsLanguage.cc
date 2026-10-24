// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/mars/MarsLanguage.h"

#include <pthread.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"
#include "eckit/value/Content.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MMapFileReader.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Parameter.h"
#include "metkit/mars/StreamWriter.h"
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
    for (const auto& param : request) {
        MarsKeyword key = param.id();
        if (key) {  // a parameter that is not a registered keyword cannot be inherited by any language
            values_[key] = param.values();
        }
    }
}

ExpansionContext& ExpansionContext::operator=(ExpansionContext&& other) noexcept {
    values_ = std::move(other.values_);
    return *this;
}

bool ExpansionContext::has(MarsKeyword key) const {
    return values_.find(key) != values_.end();
}

const std::vector<std::string>& ExpansionContext::values(MarsKeyword key) const {
    static const std::vector<std::string> empty;
    auto it = values_.find(key);
    if (it != values_.end()) {
        return it->second;
    }
    return empty;
}

void ExpansionContext::unset(MarsKeyword key) {
    values_.erase(key);
}

//----------------------------------------------------------------------------------------------------------------------

// static members of MarsLanguage
std::unique_ptr<SymbolTable<MarsVerb>> MarsLanguage::verbs_{};
std::unique_ptr<SymbolTable<MarsKeyword>> MarsLanguage::keywords_{};
std::vector<Category> MarsLanguage::categories_{};
MarsKeyword MarsLanguage::maxDataKeyword_ = 0;
std::shared_mutex MarsLanguage::contextsMutex_;
std::map<MarsVerb, size_t> MarsLanguage::langOffsets_{};
std::set<Context*, ContextPtrComparator> MarsLanguage::contextsSet_{};
std::vector<std::unique_ptr<Context>> MarsLanguage::contexts_{};
std::unique_ptr<MMapFileReader> MarsLanguage::langFile_{};

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
        langFile_ = std::make_unique<MMapFileReader>(languageBinFile.localPath());

        std::string header{langFile_->readString(4)};
        uint16_t version = langFile_->read16();

        LOG_DEBUG_LIB(LibMetkit) << "Reading language binary file header: " << header << " version: " << version
                                 << std::endl;

        if ("LANG" == header && version == LibMetkit::binaryFilesVersion()) {

            readDictionaries(*langFile_);

            uint8_t numVerbs = langFile_->read8();
            for (uint8_t u = 0; u < numVerbs; ++u) {
                MarsVerb vv     = langFile_->read8();
                uint32_t offset = langFile_->read32();
                langOffsets_.emplace(vv, offset);
            }
            ASSERT(langOffsets_.size() == numVerbs);

            // the contexts follow the verb offsets table and are shared by all the languages
            readContexts(*langFile_);

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
    categories_.clear();
    maxDataKeyword_ = 0;
    langOffsets_.clear();
    {
        std::unique_lock lock(contextsMutex_);
        contextsSet_.clear();
        contexts_.clear();
    }
    langFile_.reset();  // releases the mmap file
    return false;       // failed to load binary file - fall back to YAML parsing
}

void MarsLanguage::init() {
    if (loadBinary()) {
        return;
    }

    loadYaml();

    verbs_                   = std::make_unique<SymbolTable<MarsVerb>>();
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
                verbs_->addAlias(alias, k);
            }
        }
    }
    keywords_ = std::make_unique<SymbolTable<MarsKeyword>>();
    categories_.push_back(Category::Other);  // keyword 0
    keywords_->add(std::string{"_verb"});
    categories_.push_back(Category::Other);  // for "_verb"
    for (const std::string& a : hypercube::AxisOrder::instance().axes()) {
        keywords_->add(a);
        categories_.push_back(Category::Data);
    }
    maxDataKeyword_ = keywords_->size();
    keywords_->add(std::string{"day"});  // keyword required for date filtering by day
    categories_.push_back(Category::Derived);

    verbs_->add(std::string{"verb"});
    verbs_->add(std::string{"environ"});  // sent by mars-client to complement a request

    auto emptyValue = eckit::Value{};
    addContext(emptyValue);
}

void MarsLanguage::writeDictionaries(std::ofstream& file) {
    verbs_->write(file);
    keywords_->write(file);
    write16(file, categories_.size());
    for (const auto& category : categories_) {
        write8(file, static_cast<uint8_t>(category));
    }
    write16(file, maxDataKeyword_);
}
void MarsLanguage::readDictionaries(MMapFileReader& file) {
    verbs_                = std::make_unique<SymbolTable<MarsVerb>>(file);
    keywords_             = std::make_unique<SymbolTable<MarsKeyword>>(file);
    size_t categoriesSize = file.read16();
    categories_.clear();
    categories_.reserve(categoriesSize);
    for (size_t i = 0; i < categoriesSize; ++i) {
        categories_.push_back(static_cast<Category>(file.read8()));
    }
    maxDataKeyword_ = file.read16();
}

void MarsLanguage::writeContexts(std::ofstream& file) {
    write16(file, contexts_.size());
    for (const auto& context : contexts_) {
        context->write(file);
    }
}
void MarsLanguage::readContexts(MMapFileReader& file) {
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
        MarsKeyword key    = keywords_->symbol_or_default(keyStr);
        if (key) {  // a keyword in the modifier might apply only to a language for another verb
            keyStr = keywords_->name(key);

            auto it = types_.find(key);
            if (it != types_.end()) {
                ASSERT(categories_[key] != Category::Data || maxIndex <= key);

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
    verb_ = verbs_->symbol(verb);
    parse(verb_);
}

MarsLanguage::MarsLanguage(MarsVerb verb) : verb_(verb) {
    pthread_once(&once, initLanguage);
    parse(verb_);
}

MarsLanguage::MarsLanguage(MarsVerb verb, MMapFileReader& file) {
    pthread_once(&once, initLanguage);

    auto offset = langOffsets_.find(verb);
    ASSERT(offset != langOffsets_.end());
    file.seek(offset->second);
    verb_ = file.read8();
    ASSERT(verb_ == verb);

    uint16_t numTypes = file.read16();
    for (uint16_t i = 0; i < numTypes; ++i) {
        std::string typeName{file.readString()};
        MarsKeyword keyword = file.read16();
        types_.emplace(keyword, TypesFactory::build(typeName, keyword, file));
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

void MarsLanguage::parse(MarsVerb verb) {

    loadYaml();  // not loaded yet if the binary file is used for the other verbs

    eckit::Value lang    = languages_[verbs_->name(verb)];
    eckit::Value params  = lang.keys();
    eckit::Value options = lang["_options"];

    for (size_t i = 0; i < params.size(); ++i) {
        std::string keywordStr = params[i];
        if (keywordStr[0] == '_') {
            continue;
        }

        MarsKeyword keyword = keywords_->add(keywordStr);
        keywordList_.push_back(keywordStr);

        ASSERT(types_.find(keyword) == types_.end());

        eckit::Value settings = lang[keywordStr];

        Category category = Category::Other;
        if (settings.contains("category")) {
            std::string categoryStr = settings["category"];
            if (categoryStr == "data") {
                category = Category::Data;
            }
            else if (categoryStr == "derived") {
                category = Category::Derived;
            }
            else if (categoryStr == "postproc") {
                category = Category::PostProc;
            }
            else if (categoryStr == "sink") {
                category = Category::Sink;
            }
            else {
                std::stringstream ss;
                ss << "Keyword: " << keywordStr << " - Unknown category: " << categoryStr << " in " << settings;
                throw eckit::SeriousBug(ss.str());
            }
        }
        // categories_ is indexed by keyword id: only newly registered keywords get an entry
        if (uint16_t(keyword) == categories_.size()) {
            categories_.push_back(category);
        }

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
                keywords_->addAlias(alias, keyword);
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
            MarsKeyword key = keywords_->symbol_or_default(keywords[i]);
            if (auto iter = types_.find(key); key && iter != types_.end()) {
                iter->second->clearDefaults();
            }
        }
    }
}

Category MarsLanguage::category(MarsKeyword keyword) {
    if (!keyword || keyword >= categories_.size()) {
        throw eckit::UserError("Cannot find keyword with id " + std::to_string(keyword));
    }
    return categories_[keyword];
}

bool MarsLanguage::isData(const std::string& k) const {
    return category(keywords_->symbol(k)) == Category::Data;
}

const MarsLanguage& MarsLanguage::get(MarsVerb verb) {
    static std::shared_mutex mutex;
    static std::map<MarsVerb, MarsLanguage*> instances;
    static bool metkitForceBinfileCreation = eckit::Resource<bool>("$METKIT_FORCE_BINFILE_CREATION", false);

    {
        std::shared_lock lock(mutex);
        if (auto it = instances.find(verb); it != instances.end()) {
            return *it->second;
        }
    }

    // Upgrade to unique lock for writing
    std::unique_lock uniqueLock(mutex);
    if (auto it = instances.find(verb); it != instances.end()) {
        return *it->second;
    }

    pthread_once(&once, initLanguage);

    // load / parse the language for the given verb
    if (!metkitForceBinfileCreation && !langOffsets_.empty()) {
        if (auto it = langOffsets_.find(verb); it != langOffsets_.end()) {
            auto [newIt, inserted] = instances.emplace(verb, new MarsLanguage(verb, *langFile_));
            ASSERT(inserted);
            return *newIt->second;
        }
    }
    auto [newIt, inserted] = instances.emplace(verb, new MarsLanguage(verb));
    ASSERT(inserted);
    return *newIt->second;
}

const MarsLanguage& MarsLanguage::get(const std::string& verb) {
    pthread_once(&once, initLanguage);
    return get(verbs_->symbol(eckit::StringTools::lower(verb)));
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
    return verbs_->name(verbs_->symbol(eckit::StringTools::lower(verb)));
}

class TypeHidden : public Type {

    static const std::string typeName;

    bool flatten() const override { return false; }
    void print(std::ostream& out) const override { out << "TypeHidden"; }
    bool expand(std::string&, const MarsRequest&) const override { return true; }

public:

    TypeHidden() : Type(typeName, MarsLanguage::addKeyword(typeName), eckit::Value()) {}
    ~TypeHidden() override = default;
};

const std::string TypeHidden::typeName{"hidden"};

std::optional<std::reference_wrapper<const Type>> MarsLanguage::type(MarsKeyword key) const noexcept {
    auto it = types_.find(key);
    if (it != types_.end()) {
        return *it->second;
    }
    return std::nullopt;
}

const Type& MarsLanguage::typeRef(const std::string& name) const {
    MarsKeyword key = keywords_->symbol_or_default(name);
    if (key && types_.find(key) != types_.end()) {
        return type(key).value().get();
    }
    if (!name.empty() && name[0] == '_') {  // internal keywords (e.g. _verb) are known, but have no type in a language
        static const TypeHidden hidden{};
        return hidden;
    }

    throw eckit::UserError("Cannot find a type for '" + name + "'");
}
const Type* MarsLanguage::type(const std::string& name) const {
    MarsKeyword key = keywords_->symbol_or_default(name);
    if (key && types_.find(key) != types_.end()) {
        return &type(key).value().get();
    }
    if (!name.empty() && name[0] == '_') {  // internal keywords (e.g. _verb) are known, but have no type in a language
        static const TypeHidden hidden{};
        return &hidden;
    }

    return nullptr;
}

MarsRequest MarsLanguage::expand(const MarsRequest& r, ExpansionContext& ctx, bool inherit, bool strict) const {
    MarsRequest result(verb_);

    try {
        // Resolve the keywords of the request in this language. They are processed sorted by keyword (axes first,
        // in axis order), so the order of the parameters in the request does not matter.
        std::map<MarsKeyword, const Parameter*> paramSet;

        for (const auto& param : r) {
            // a typed parameter that is already a keyword of this language does not need to be looked up
            MarsKeyword key = param.typed() ? param.id() : MarsKeyword{};
            if (!key || types_.find(key) == types_.end()) {
                std::string p = eckit::StringTools::lower(param.name());
                key           = keywords_->symbol_or_default(p);
                if (!key || types_.find(key) == types_.end()) {
                    // not a keyword of this language (the dictionary is shared by all the verbs):
                    // fall back to fuzzy matching, governed by METKIT_LANGUAGE_STRICT_MODE
                    p   = bestMatch(p, keywordList_, true, false, true, aliases_);
                    key = keywords_->symbol_or_default(p);
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
                if (s == "all" && type(k).value().get().multiple()) {
                    result.setValuesTyped(type(k).value().get(), std::vector<std::string>{"all"});
                    continue;
                }
            }

            auto& t = type(k).value().get();
            t.expand(values, result);
            result.setValuesTyped(t, values);
            t.check(values);
        }

        if (inherit) {
            for (const auto& t : types_) {
                if (result.countValues(t.first) == 0) {
                    if (ctx.has(t.first)) {
                        result.setValuesTyped(*t.second, ctx.values(t.first));
                    }
                    else {
                        t.second->setDefaults(result);
                    }
                }
            }
        }

        // pass2 modifies the request: go through a snapshot of the types, not through the parameters being modified
        for (const auto& param : result) {
            param.type().pass2(result);
        }

        for (const auto& t : types_) {
            t.second->finalise(result, strict);
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

void MarsLanguage::flatten(const MarsRequest& request, const std::vector<MarsKeyword>& keys, size_t i,
                           MarsRequest& result, FlattenCallback& callback) const {
    if (i == keys.size()) {
        callback(result);
        return;
    }

    MarsKeyword key = keys[i];

    // internal keywords (e.g. _verb) are registered, but have no type in the language: look them up by name
    auto it       = types_.find(key);
    const Type& t = it != types_.end() ? *it->second : typeRef(name(key));
    if (!t.flatten()) {
        flatten(request, keys, i + 1, result, callback);
        return;
    }

    const std::vector<std::string>& values = t.flattenValues(request);

    for (std::vector<std::string>::const_iterator j = values.begin(); j != values.end(); ++j) {
        result.setValue(key, *j);
        flatten(request, keys, i + 1, result, callback);
    }
}

void MarsLanguage::flatten(const MarsRequest& request, FlattenCallback& callback) const {
    MarsRequest result(request);
    std::vector<MarsKeyword> params;
    for (const auto& p : request) {
        MarsKeyword key = p.id();
        if (!key) {
            throw eckit::UserError("Cannot flatten a request with an unknown keyword '" + p.name() + "'");
        }
        params.push_back(key);
    }
    flatten(request, params, 0, result, callback);
}

MarsVerb MarsLanguage::verb(const std::string& name) {
    pthread_once(&once, initLanguage);
    return verbs_->symbol(name);
}
MarsVerb MarsLanguage::verb_or_default(const std::string& name) noexcept {
    pthread_once(&once, initLanguage);
    return verbs_->symbol_or_default(name);
}
const std::string& MarsLanguage::name(MarsVerb verb) {
    pthread_once(&once, initLanguage);
    return verbs_->name(verb);
}

MarsKeyword MarsLanguage::addKeyword(const std::string& name, Category category) {
    pthread_once(&once, initLanguage);
    MarsKeyword key = keywords_->add(name);
    categories_.push_back(category);
    return key;
}
MarsKeyword MarsLanguage::keyword(const std::string& name) {
    pthread_once(&once, initLanguage);
    return keywords_->symbol(name);
}
MarsKeyword MarsLanguage::keyword_or_default(const std::string& name) noexcept {
    pthread_once(&once, initLanguage);
    return keywords_->symbol_or_default(name);
}
const std::string& MarsLanguage::name(MarsKeyword keyword) {
    pthread_once(&once, initLanguage);
    return keywords_->name(keyword);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
