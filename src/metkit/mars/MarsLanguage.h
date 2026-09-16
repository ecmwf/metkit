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

enum class MetadataGroup : uint8_t {
    None = 0,
    Data,
    Derived,
    PostProc,
    Sink
};

//----------------------------------------------------------------------------------------------------------------------

struct MetadataType {
    MetadataGroup group{MetadataGroup::None};
    Type* type;
};

//----------------------------------------------------------------------------------------------------------------------
class MarsLanguageRegistry;

class MarsLanguage : private eckit::NonCopyable {

public:  // methods

    MarsLanguage(const std::string& verb);
    // MarsLanguage(const MarsLanguage&);

    ~MarsLanguage();

    MarsRequest expand(const MarsRequest& r, MarsRequest& ctx, bool inherit, bool strict) const;

    const std::string& verb() const;

    void flatten(const MarsRequest& request, FlattenCallback& callback) const;

    Type* type(const std::string& name) const;

    bool isData(const std::string& keyword) const;
    bool isDerived(const std::string& keyword) const;
    bool isPostProc(const std::string& keyword) const;
    bool isSink(const std::string& keyword) const;

public:  // class methods

    static std::string expandVerb(const std::string& verb);

    [[ deprecated("Fuzzy matching is replaced by exact matching + alias support") ]]
    static std::string bestMatch(const std::string& name, const std::vector<std::string>& values, bool fail, bool quiet,
                                 bool fullMatch, const std::map<std::string, std::string>& aliases = {});

    static eckit::Value jsonFile(const std::string& name);


private:  // methods

    MetadataGroup group(const std::string& keyword) const;
    void flatten(const MarsRequest& request, const std::vector<std::string>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback) const;
    void parseModifier(ModifierType typ, std::shared_ptr<Context> ctx, size_t maxIndex, const eckit::Value& mod);

private:  // members

    std::string verb_;
    std::unordered_map<std::string, MetadataType> types_;
    std::vector<std::pair<std::string, Type*>> typesByAxisOrder_;
    std::unordered_map<std::string, std::string> aliases_;

    mutable std::unordered_map<std::string, std::string> cache_;
};

//----------------------------------------------------------------------------------------------------------------------

// registry for MarsLanguage instances, which are lazy loaded from YAML file, or binary file, if available.
class MarsLanguageRegistry {
public:

    static MarsLanguageRegistry& instance();

    const MarsLanguage& language(const std::string& verb) const;

private:

    mutable std::mutex mutex_;
    mutable std::unordered_map<std::string, MarsLanguage*> languages_;
};

}  // namespace metkit::mars
