/*
 * (C) Copyright 1996-2013 ECMWF.
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

//----------------------------------------------------------------------------------------------------------------------

class MarsLanguage : private eckit::NonCopyable {
public:

    MarsLanguage(const std::string& verb);
    ~MarsLanguage();

    MarsRequest expand(const MarsRequest& r) const;

    typedef std::map<std::string, Type* >::iterator iterator;

    iterator begin() { return types_.begin(); }
    iterator end() { return types_.end(); }

    void set(const std::string& name, const std::vector<std::string>& values);

    const std::string& verb() const;

    void flatten(const MarsRequest& request);


// - Class methds

    static std::string expandVerb(const std::string& verb);

private:
// -- Contructors

    std::string verb_;
    std::map<std::string, Type* > types_;
    std::vector<std::string> keywords_;;

private: // Methods
    std::string expandKeyword(const std::string& keyword);
    void expandValues(const std::string& keyword, const eckit::Value& language, std::vector<std::string>& values) const;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
