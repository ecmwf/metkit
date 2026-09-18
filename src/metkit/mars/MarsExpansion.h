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
#include <string>
#include <vector>

#include "metkit/mars/MarsParsedRequest.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

class ExpansionContext;

//----------------------------------------------------------------------------------------------------------------------

class FlattenCallback {
public:

    virtual void operator()(const MarsRequest&) = 0;

protected:

    virtual ~FlattenCallback() = default;
};

class ExpandCallback {
public:

    virtual void operator()(const MarsRequest&) = 0;

protected:

    virtual ~ExpandCallback() = default;
};

//----------------------------------------------------------------------------------------------------------------------

class MarsExpansion {
public:

    MarsExpansion(bool inherit, bool strict = false);

    void reset();

    MarsRequest expand(const MarsRequest&);
    std::vector<MarsRequest> expand(const std::vector<MarsParsedRequest>&);
    std::vector<MarsRequest> expand(const std::vector<MarsRequest>&);

    void expand(const MarsRequest&, ExpandCallback&);
    void flatten(const MarsRequest&, FlattenCallback&);

private:

    ExpansionContext& ctxForVerb(const std::string& verb);

private:

    bool inherit_;
    bool strict_;

    std::map<std::string, ExpansionContext*> ctx_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
