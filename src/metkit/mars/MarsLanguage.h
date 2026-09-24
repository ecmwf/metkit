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

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsParsedRequest.h"
#include "metkit/mars/MarsRequest.h"
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

class MarsLanguage {

public:  // methods

    MarsLanguage(Verb verb);
    MarsLanguage(const std::string& verb);
    ~MarsLanguage();

    MarsLanguage(const MarsLanguage&)            = delete;
    MarsLanguage(MarsLanguage&&)                 = delete;
    MarsLanguage& operator=(const MarsLanguage&) = delete;
    MarsLanguage& operator=(MarsLanguage&&)      = delete;

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
    static const std::string& name(Keyword keyword);

    static void init();

private:  // methods

    void parse(Verb verb);

    Category group(Keyword keyword) const;
    void flatten(const MarsRequest& request, const std::vector<std::string>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback) const;
    void parseModifier(ModifierType typ, std::shared_ptr<Context> ctx, size_t maxIndex, const eckit::Value& mod);

private:  // members

    static Dictionary<Verb> verbs_;
    static Dictionary<Keyword> keywords_;

    Verb verb_;
    mutable std::unordered_map<Keyword, Type*> types_;
    std::vector<std::pair<Keyword, Type*>> typesByAxisOrder_;

    // for bestMatch - to be removed as we turn off fuzzy matching
    std::vector<std::string> keywordList_;
    std::map<std::string, std::string> aliases_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
