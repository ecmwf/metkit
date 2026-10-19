// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include <cstddef>
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
#include "metkit/mars/Serialize.h"
#include "metkit/mars/SymbolTable.h"
#include "metkit/mars/Type.h"

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

class MarsLanguage {
public:  // methods

    MarsLanguage(MarsVerb verb);
    MarsLanguage(const std::string& verb);
    MarsLanguage(MarsVerb verb, MemFile& file);
    ~MarsLanguage() = default;

    MarsLanguage(const MarsLanguage&)            = delete;
    MarsLanguage(MarsLanguage&&)                 = delete;
    MarsLanguage& operator=(const MarsLanguage&) = delete;
    MarsLanguage& operator=(MarsLanguage&&)      = delete;

    MarsVerb verb() const { return verb_; }

    void write(std::ofstream& file) const;

    MarsRequest expand(const MarsRequest& r, ExpansionContext& ctx, bool inherit, bool strict) const;

    void flatten(const MarsRequest& request, FlattenCallback& callback) const;

    std::shared_ptr<const Type> type(MarsKeyword name) const;
    const Type* type(const std::string& name) const;

    bool isData(MarsKeyword keyword) const;
    bool isDerived(MarsKeyword keyword) const;
    bool isPostProc(MarsKeyword keyword) const;
    bool isSink(MarsKeyword keyword) const;

    bool isData(const std::string& keyword) const;
    bool isDerived(const std::string& keyword) const;
    bool isPostProc(const std::string& keyword) const;
    bool isSink(const std::string& keyword) const;

public:  // class methods

    static const std::string& expandVerb(const std::string& verb);

    static const MarsLanguage& get(MarsVerb verb);
    static const MarsLanguage& get(const std::string& verb);

    static std::string bestMatch(const std::string& name, const std::vector<std::string>& values, bool fail, bool quiet,
                                 bool fullMatch, const std::map<std::string, std::string>& aliases = {});

    static eckit::Value jsonFile(const std::string& name);

    static MarsVerb verb(const std::string& name);
    static MarsVerb hasVerb(const std::string& name);
    static const std::string& name(MarsVerb verb);

    static MarsKeyword addKeyword(const std::string& name);
    static MarsKeyword hasKeyword(const std::string& name);
    static MarsKeyword keyword(const std::string& name);
    static MarsKeyword maxDataKeyword() { return maxDataKeyword_; }
    static const std::string& name(MarsKeyword keyword);

    static void init();

    static void writeDictionaries(std::ofstream& file);
    static void readDictionaries(MemFile& file);

    static void writeContexts(std::ofstream& file);
    static void readContexts(MemFile& file);

private:  // methods

    friend class Type;
    friend class TypeMixed;

    void parse(MarsVerb verb);

    static bool loadBinary();  // true if the language was loaded from the binary file
    static void loadYaml();

    /// Whether a type takes part in the defaults and finalisation steps of the expansion, which process the types in
    /// the order of types_: the axes and all the types that are not data. Data keywords that are not axes (e.g. tile)
    /// do not.
    bool isOrdered(MarsKeyword key, const Type& type) const {
        return key < maxDataKeyword_ || type.category() != Category::Data;
    }

    Category category(MarsKeyword keyword) const;
    void flatten(const MarsRequest& request, const std::vector<MarsKeyword>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback) const;
    void parseModifier(ModifierType typ, const Context& ctx, size_t maxIndex, const eckit::Value& mod);
    static const Context& context(size_t ctxId);
    static const Context& addContext(eckit::Value& val);

private:  // members

    static std::unique_ptr<SymbolTable<MarsVerb>> verbs_;
    static std::unique_ptr<SymbolTable<MarsKeyword>> keywords_;
    static MarsKeyword maxDataKeyword_;

    static std::shared_mutex contextsMutex_;
    static std::set<Context*, ContextPtrComparator> contextsSet_;
    static std::vector<std::unique_ptr<Context>> contexts_;

    static std::map<MarsVerb, size_t> langOffsets_;
    static MemFile langFile_;

    MarsVerb verb_;
    /// The types of the language, sorted by keyword. The keywords of the axes are registered following the AxisOrder
    /// (see init()), so iterating over the types visits the axes in axis order, followed by all the other keywords.
    std::map<MarsKeyword, std::shared_ptr<Type>> types_;

    // for bestMatch - to be removed as we turn off fuzzy matching
    std::vector<std::string> keywordList_;
    std::map<std::string, std::string> aliases_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
