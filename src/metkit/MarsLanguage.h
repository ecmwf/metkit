/*
 * (C) Copyright 1996-2017 ECMWF.
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

#include "metkit/MarsRequest.h"
#include "eckit/memory/NonCopyable.h"


namespace metkit {

class Type;
class FlattenCallback;

//----------------------------------------------------------------------------------------------------------------------

class ExpandContext {
public:
    virtual void print(std::ostream&) const = 0;
};


//----------------------------------------------------------------------------------------------------------------------


class MarsLanguage : private eckit::NonCopyable {

    typedef std::map<std::string, std::string> StringMap;

public: // methods

    MarsLanguage(const std::string& verb);

    ~MarsLanguage();

    MarsRequest expand(const MarsRequest& r, bool inherit);

    void reset();

    const std::string& verb() const;

    void flatten(const MarsRequest& request, FlattenCallback& callback);

public: // class methods

    static std::string expandVerb(const std::string& verb);

    static std::string bestMatch(const std::string& what,
                                 const std::vector<std::string>& values,
                                 bool fail,
                                 bool quiet,
                                 const StringMap& aliases = StringMap(),
                                 const ExpandContext* = 0);

    static eckit::Value jsonFile(const std::string& name);


private: // methods

    void flatten(const MarsRequest& request,
                 const std::vector<std::string>& params,
                 size_t i,
                 MarsRequest& result,
                 FlattenCallback& callback);

    Type* type(const std::string& name) const;

private: // members

    std::string verb_;
    std::map<std::string, Type* > types_;
    std::vector<std::string> keywords_;

    StringMap aliases_;

    mutable StringMap cache_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
