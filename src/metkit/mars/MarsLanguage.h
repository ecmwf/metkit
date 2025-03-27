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

#ifndef metkit_MarsLanguage_H
#define metkit_MarsLanguage_H

#include "eckit/memory/NonCopyable.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit {
namespace mars {

class Type;
class FlattenCallback;
class MarsExpandContext;

//----------------------------------------------------------------------------------------------------------------------


class MarsLanguage : private eckit::NonCopyable {

    typedef std::map<std::string, std::string> StringMap;

public:  // methods

    MarsLanguage(const std::string& verb);

    ~MarsLanguage();

    MarsRequest expand(const MarsExpandContext& ctx, const MarsRequest& r, bool inherit, bool strict);

    void reset();

    const std::string& verb() const;

    void flatten(const MarsExpandContext& ctx, const MarsRequest& request, FlattenCallback& callback);

    static eckit::PathName languageYamlFile();

    Type* type(const std::string& name) const;

    bool isData(const std::string& keyword) const;

public:  // class methods

    static std::string expandVerb(const MarsExpandContext&, const std::string& verb);

    static std::string bestMatch(const MarsExpandContext& ctx, const std::string& what,
                                 const std::vector<std::string>& values, bool fail, bool quiet, bool fullMatch,
                                 const StringMap& aliases = StringMap());

    static eckit::Value jsonFile(const std::string& name);


private:  // methods

    void flatten(const MarsRequest& request, const std::vector<std::string>& params, size_t i, MarsRequest& result,
                 FlattenCallback& callback);

private:  // members

    std::string verb_;
    std::map<std::string, Type*> types_;
    std::set<std::string> dataKeywords_;
    std::vector<std::pair<std::string, Type*>> typesByAxisOrder_;
    std::vector<std::string> keywords_;

    StringMap aliases_;

    mutable StringMap cache_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
