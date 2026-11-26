// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include <array>
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "metkit/mars/MarsKeyword.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/MarsVerb.h"
#include "metkit/mars/SymbolTable.h"
#include "metkit/mars/Type.h"

class MMapFileReader;

namespace metkit::mars {

class Context;
class FlattenCallback;

//----------------------------------------------------------------------------------------------------------------------

enum class ModifierType {
    DEFAULT,
    SET,
    UNSET
};

//----------------------------------------------------------------------------------------------------------------------

class ExpansionContext {
public:

    ExpansionContext() = default;
    ExpansionContext(const MarsRequest& request);
    ExpansionContext& operator=(ExpansionContext&& other) noexcept;

    bool has(MarsKeyword key) const;

    const std::vector<std::string>& values(MarsKeyword key) const;

    void unset(MarsKeyword key);
    void unset(const std::string& key);

private:

    std::unordered_map<MarsKeyword, std::vector<std::string>> values_;
};

//----------------------------------------------------------------------------------------------------------------------

struct ContextPtrComparator {
    bool operator()(const Context* a, const Context* b) const { return std::less<const Context>()(*a, *b); }
};

/// The language of a verb. Languages are only obtained from MarsLanguage::get(), which keeps them for the lifetime of
/// the process: the typed parameters of requests refer to the types of a language without owning them.
class MarsLanguage {
public:  // methods

    ~MarsLanguage() = default;

    MarsLanguage(const MarsLanguage&)            = delete;
    MarsLanguage(MarsLanguage&&)                 = delete;
    MarsLanguage& operator=(const MarsLanguage&) = delete;
    MarsLanguage& operator=(MarsLanguage&&)      = delete;

    MarsVerb verb() const { return verb_; }

    void write(std::ofstream& file) const;

    MarsRequest expand(const MarsRequest& r, ExpansionContext& ctx, bool inherit, bool strict) const;

    void flatten(const MarsRequest& request, FlattenCallback& callback) const;

    std::optional<std::reference_wrapper<const Type>> type(MarsKeyword key) const noexcept;

    [[deprecated("use typeRef returning Type reference")]]
    const Type* type(const std::string& name) const;
    const Type& typeRef(const std::string& name) const;

    [[deprecated("use category(MarsKeyword) instead")]]
    bool isData(const std::string& keyword) const;

public:  // class methods

    static const std::string& expandVerb(const std::string& verb);

    static const MarsLanguage& get(MarsVerb verb);
    static const MarsLanguage& get(const std::string& verb);
    /// @return false for the verbs that are registered without a language definition (e.g. "environ"): get() throws
    static bool hasLanguage(MarsVerb verb);

    static std::string bestMatch(const std::string& name, const std::vector<std::string>& values, bool fail, bool quiet,
                                 bool fullMatch, const std::map<std::string, std::string>& aliases = {});

    static eckit::Value jsonFile(const std::string& name);

    static MarsVerb verb(const std::string& name);
    static MarsVerb verb_or_default(const std::string& name) noexcept;
    static const std::string& name(MarsVerb verb);

    static MarsKeyword addKeyword(const std::string& name, Category category = Category::Other);
    static MarsKeyword keyword(const std::string& name);
    static MarsKeyword keyword_or_default(const std::string& name) noexcept;
    static Category category(MarsKeyword keyword);
    static MarsKeyword maxDataKeyword() { return maxDataKeyword_; }
    static const std::string& name(MarsKeyword keyword);

    static void init();

    static void writeDictionaries(std::ofstream& file);
    static void readDictionaries(MMapFileReader& file);

    static void writeContexts(std::ofstream& file);
    static void readContexts(MMapFileReader& file);

private:  // methods

    explicit MarsLanguage(MarsVerb verb);
    MarsLanguage(const std::string& verb);
    MarsLanguage(MarsVerb verb, MMapFileReader& file);

    friend class Type;
    friend class TypeMixed;

    void parse(MarsVerb verb);

    /// Registers @p name with @p category if it is a new keyword (the category of a known keyword is left unchanged)
    static MarsKeyword registerKeyword(const std::string& name, Category category);
    /// The category of @p keyword in a language definition (Category::Other if not specified)
    static Category keywordCategory(const std::string& keyword, const eckit::Value& settings);

    static bool loadBinary();  // true if the language was loaded from the binary file
    static void loadYaml();

    void flatten(const MarsRequest& request, const std::vector<MarsKeyword>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback) const;
    void parseModifier(ModifierType typ, const Context& ctx, size_t maxIndex, const eckit::Value& mod);
    static const Context& context(size_t ctxId);
    static const Context& addContext(eckit::Value& val);

private:  // members

    static std::unique_ptr<SymbolTable<MarsVerb>> verbs_;
    static std::unique_ptr<SymbolTable<MarsKeyword>> keywords_;
    /// Indexed by keyword, never resized: an entry is written once, under the lock of keywords_, before its keyword is
    /// published, so category() can read it without locking. Unregistered keywords are Category::Unknown.
    static std::array<Category, size_t{std::numeric_limits<MarsKeyword::value_t>::max()} + 1> categories_;
    static MarsKeyword maxDataKeyword_;

    static std::shared_mutex contextsMutex_;
    static std::set<Context*, ContextPtrComparator> contextsSet_;
    static std::vector<std::unique_ptr<Context>> contexts_;

    static std::map<MarsVerb, size_t> langOffsets_;
    static std::unique_ptr<MMapFileReader> langFile_;

    MarsVerb verb_;
    /// The types of the language, sorted by keyword. The keywords of the axes are registered following the AxisOrder
    /// (see init()), so iterating over the types visits the axes in axis order, followed by all the other keywords.
    std::map<MarsKeyword, std::unique_ptr<Type>> types_;

    // for bestMatch - to be removed as we turn off fuzzy matching
    std::vector<std::string> keywordList_;
    std::map<std::string, std::string> aliases_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
