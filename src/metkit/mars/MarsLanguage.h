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

#include "eckit/memory/NonCopyable.h"

#include "metkit/mars/MarsRequest.h"


namespace metkit::mars {

class Context;
class FlattenCallback;
class Type;

//----------------------------------------------------------------------------------------------------------------------

enum class ModifierType {
    DEFAULT,
    SET,
    UNSET
};

//----------------------------------------------------------------------------------------------------------------------

class MarsLanguage : private eckit::NonCopyable {

public:  // methods

    MarsLanguage(const std::string& verb);

    ~MarsLanguage();

    MarsRequest expand(const MarsRequest& r, bool inherit, bool strict);

    void reset();

    const std::string& verb() const;

    void flatten(const MarsRequest& request, FlattenCallback& callback);

    static eckit::PathName languageYamlFile();

    Type* type(const std::string& name) const;

    /// @brief Parse/normalise the values of a single keyword, in isolation from a full request.
    /// Resolves @p keyword against this verb's language (aliases and partial matches allowed),
    /// then expands @p values using the keyword's Type. Range syntax (e.g. "1/to/10/by/1") and
    /// per-key normalisation (e.g. date/time) are applied. Context-sensitive keys (e.g. those
    /// backed by TypeMixed) consult @p context; supply the relevant neighbouring keys there.
    /// @note This performs the single-pass expansion only. Second-pass resolution (pass2/finalise,
    /// e.g. rule-based 'param' expansion) and default inheritance are not applied here; use a
    /// (scoped) MarsRequest expansion for those.
    /// @param keyword keyword to parse (canonical, alias or unambiguous prefix)
    /// @param values values to expand (already split on '/')
    /// @param context other keys providing context for context-sensitive types
    /// @param strict if true, validate expanded values and throw on invalid values
    /// @return expanded/normalised values
    std::vector<std::string> expandKey(const std::string& keyword, std::vector<std::string> values,
                                      const MarsRequest& context = {}, bool strict = false);

    bool isData(const std::string& keyword) const;

    bool isPostProc(const std::string& keyword) const;

    bool isSink(const std::string& keyword) const;
    const std::set<std::string>& sinkKeywords() const;

public:  // class methods

    static std::string expandVerb(const std::string& verb);

    static std::string bestMatch(const std::string& name, const std::vector<std::string>& values, bool fail, bool quiet,
                                 bool fullMatch, const std::map<std::string, std::string>& aliases = {});

    static eckit::Value jsonFile(const std::string& name);


private:  // methods

    void flatten(const MarsRequest& request, const std::vector<std::string>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback);
    void parseModifier(ModifierType typ, std::shared_ptr<Context> ctx, size_t maxIndex, const eckit::Value& mod);

private:  // members

    std::string verb_;
    std::map<std::string, Type*> types_;
    std::set<std::string> dataKeywords_;
    std::set<std::string> sinkKeywords_;
    std::set<std::string> postProcKeywords_;
    std::vector<std::pair<std::string, Type*>> typesByAxisOrder_;
    std::vector<std::string> keywords_;

    std::map<std::string, std::string> aliases_;

    mutable std::map<std::string, std::string> cache_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
