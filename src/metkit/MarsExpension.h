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

#ifndef metkit_MarsExpension_H
#define metkit_MarsExpension_H

#include "metkit/MarsRequest.h"
#include "eckit/memory/NonCopyable.h"


namespace metkit {

class MarsLanguage;

class FlattenCallback {
public:
    virtual void operator()(const MarsRequest&) = 0;
};

class FlattenFilter {
public:
    virtual bool operator()(const std::string& keyword,
                            std::vector<std::string>& values,
                            const MarsRequest& request) = 0;
};

//----------------------------------------------------------------------------------------------------------------------

class MarsExpension : public eckit::NonCopyable {
public:
// -- Contructors

    MarsExpension();
    ~MarsExpension();

    std::vector<MarsRequest> operator()(const std::vector<MarsRequest>&);
    void flatten(const MarsRequest& request, FlattenCallback& callback, FlattenFilter& filter);

private: // members

    MarsLanguage& language(const std::string& verb);

    std::map<std::string, MarsLanguage*> languages_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
