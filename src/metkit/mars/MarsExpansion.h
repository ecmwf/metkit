// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include <map>
#include <string>
#include <vector>

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParsedRequest.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

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

    std::map<std::string, ExpansionContext> ctx_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
