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
#include <set>
#include <string>
#include <vector>

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

    bool has(const std::string& key) const;
    const std::vector<std::string>& values(const std::string& key) const;
    void unset(const std::string& key);

private:

    std::map<std::string, std::vector<std::string>> values_;
};

class MarsLanguage {

public:  // methods

    MarsLanguage(const std::string& verb);
    ~MarsLanguage();

    MarsLanguage(const MarsLanguage&)            = delete;
    MarsLanguage(MarsLanguage&&)                 = delete;
    MarsLanguage& operator=(const MarsLanguage&) = delete;
    MarsLanguage& operator=(MarsLanguage&&)      = delete;

    MarsRequest expand(const MarsRequest& r, ExpansionContext& ctx, bool inherit, bool strict) const;

    const std::string& verb() const;

    void flatten(const MarsRequest& request, FlattenCallback& callback) const;

    const Type* type(const std::string& name) const;

    bool isData(const std::string& keyword) const;
    bool isDerived(const std::string& keyword) const;
    bool isPostProc(const std::string& keyword) const;
    bool isSink(const std::string& keyword) const;

    /// @brief Whether `keyword` is a keyword of this verb, or an alias of one
    bool isKeyword(const std::string& keyword) const;

public:  // class methods

    static std::string expandVerb(const std::string& verb);
    static const MarsLanguage& get(const std::string& verb);

    static std::string bestMatch(const std::string& name, const std::vector<std::string>& values, bool fail, bool quiet,
                                 bool fullMatch, const std::map<std::string, std::string>& aliases = {});

    static eckit::Value jsonFile(const std::string& name);


private:  // methods

    Category category(const std::string& keyword) const;
    void flatten(const MarsRequest& request, const std::vector<std::string>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback) const;
    void parseModifier(ModifierType typ, std::shared_ptr<Context> ctx, size_t maxIndex, const eckit::Value& mod);

private:  // members

    std::string verb_;
    std::map<std::string, Type*> types_;
    std::vector<std::pair<std::string, Type*>> typesByAxisOrder_;

    // for bestMatch - to be removed as we turn off fuzzy matching
    std::vector<std::string> keywords_;
    std::map<std::string, std::string> aliases_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
