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


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class MarsLanguage {
public:

    MarsLanguage(const std::string& verb);

    MarsRequest expand(const MarsRequest& r) const;

    typedef std::map<std::string, std::vector<std::string> >::iterator iterator;

    iterator begin() { return inheritence_.begin(); }
    iterator end() { return inheritence_.end(); }

    void set(const std::string& name, const std::vector<std::string>& values);

    const std::string& verb() const;

// - Class methds

    static std::string expandVerb(const std::string& verb);

private:
// -- Contructors

    std::string verb_;

    eckit::Value lang_;

    std::map<std::string, std::vector<std::string> > inheritence_;

private: // Methods
    std::string expandKeyword(const std::string& keyword);
    void expandValues(const std::string& keyword, const eckit::Value& language, std::vector<std::string>& values) const;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
