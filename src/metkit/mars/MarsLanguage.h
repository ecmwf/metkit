/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Serialize.h"
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
    ExpansionContext& operator=(ExpansionContext&& other);

    bool has(Keyword key) const;

    const std::vector<std::string>& values(Keyword key) const;

    void unset(Keyword key);
    void unset(const std::string& key);

private:

    std::unordered_map<Keyword, std::vector<std::string>> values_;
};

//----------------------------------------------------------------------------------------------------------------------

struct ContextPtrComparator {
    bool operator()(const Context* a, const Context* b) const { return std::less<const Context>()(*a, *b); }
};

class MarsLanguage {

public:  // methods

    MarsLanguage(Verb verb);
    MarsLanguage(const std::string& verb);
    MarsLanguage(Verb verb, MemFile& file);
    ~MarsLanguage() = default;

    MarsLanguage(const MarsLanguage&)            = delete;
    MarsLanguage(MarsLanguage&&)                 = delete;
    MarsLanguage& operator=(const MarsLanguage&) = delete;
    MarsLanguage& operator=(MarsLanguage&&)      = delete;

    void write(std::ofstream& file) const;

    MarsRequest expand(const MarsRequest& r, ExpansionContext& ctx, bool inherit, bool strict) const;

    void flatten(const MarsRequest& request, FlattenCallback& callback) const;

    const Type* type(Keyword name) const;
    const Type* type(const std::string& name) const;

    bool isData(Keyword keyword) const;
    bool isDerived(Keyword keyword) const;
    bool isPostProc(Keyword keyword) const;
    bool isSink(Keyword keyword) const;

    bool isData(const std::string& keyword) const;
    bool isDerived(const std::string& keyword) const;
    bool isPostProc(const std::string& keyword) const;
    bool isSink(const std::string& keyword) const;

    /// @brief Whether `keyword` is a keyword of this verb, or an alias of one
    bool isKeyword(const std::string& keyword) const;

public:  // class methods

    static const std::string& expandVerb(const std::string& verb);

    static const MarsLanguage& get(Verb verb);
    static const MarsLanguage& get(const std::string& verb);

    static std::string bestMatch(const std::string& name, const std::vector<std::string>& values, bool fail, bool quiet,
                                 bool fullMatch, const std::map<std::string, std::string>& aliases = {});

    static eckit::Value jsonFile(const std::string& name);

    static Verb verb(const std::string& name);
    static const std::string& name(Verb verb);

    static Keyword addKeyword(const std::string& name);
    static Keyword hasKeyword(const std::string& name);
    static Keyword keyword(const std::string& name);
    static Keyword maxDataKeyword() { return maxDataKeyword_; }
    static const std::string& name(Keyword keyword);

    static void init();

    static void writeDictionaries(std::ofstream& file);
    static void readDictionaries(MemFile& file);

    static void writeContexts(std::ofstream& file);
    static void readContexts(MemFile& file);

private:  // methods

    friend class Type;
    friend class TypeMixed;

    void parse(Verb verb);

    static bool loadBinary();  // true if the language was loaded from the binary file
    static void loadYaml();

    /// Whether a type takes part in the defaults and finalisation steps of the expansion, which process the types in
    /// the order of types_: the axes and all the types that are not data. Data keywords that are not axes (e.g. tile)
    /// do not.
    bool isOrdered(Keyword key, const Type& type) const {
        return key < maxDataKeyword_ || type.category() != Category::Data;
    }

    Category category(Keyword keyword) const;
    void flatten(const MarsRequest& request, const std::vector<Keyword>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback) const;
    void parseModifier(ModifierType typ, const Context& ctx, size_t maxIndex, const eckit::Value& mod);
    static const Context& context(size_t ctxId);
    static const Context& addContext(eckit::Value& val);

private:  // members

    static std::unique_ptr<Dictionary<Verb>> verbs_;
    static std::unique_ptr<Dictionary<Keyword>> keywords_;
    static Keyword maxDataKeyword_;

    static std::shared_mutex contextsMutex_;
    static std::set<Context*, ContextPtrComparator> contextsSet_;
    static std::vector<std::unique_ptr<Context>> contexts_;

    static std::map<Keyword, size_t> langOffsets_;
    static MemFile langFile_;

    Verb verb_;
    /// The types of the language, sorted by keyword. The keywords of the axes are registered following the AxisOrder
    /// (see init()), so iterating over the types visits the axes in axis order, followed by all the other keywords.
    std::map<Keyword, std::shared_ptr<Type>> types_;

    // for bestMatch - to be removed as we turn off fuzzy matching
    std::vector<std::string> keywordList_;
    std::map<std::string, std::string> aliases_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
