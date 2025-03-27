/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <list>
#include <set>

#include "eckit/config/Resource.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/types/Types.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

FlattenCallback::~FlattenCallback() {}

ExpandCallback::~ExpandCallback() {}

//----------------------------------------------------------------------------------------------------------------------

MarsExpension::MarsExpension(bool inherit, bool strict) : inherit_(inherit), strict_(strict) {}

MarsExpension::~MarsExpension() {
    for (std::map<std::string, MarsLanguage*>::iterator j = languages_.begin(); j != languages_.end(); ++j) {
        delete (*j).second;
    }
}

void MarsExpension::reset() {
    for (std::map<std::string, MarsLanguage*>::iterator j = languages_.begin(); j != languages_.end(); ++j) {
        (*j).second->reset();
    }
}


MarsLanguage& MarsExpension::language(const MarsExpandContext& ctx, const std::string& verb) {

    std::string v = MarsLanguage::expandVerb(ctx, verb);

    std::map<std::string, MarsLanguage*>::iterator j = languages_.find(v);
    if (j == languages_.end()) {
        languages_.insert(std::make_pair(v, new MarsLanguage(v)));
        j = languages_.find(v);
    }
    return *(*j).second;
}


std::vector<MarsRequest> MarsExpension::expand(const std::vector<MarsParsedRequest>& requests) {
    std::vector<MarsRequest> result;

    // Implement inheritence
    for (auto j = requests.begin(); j != requests.end(); ++j) {

        MarsLanguage& lang = language((*j), (*j).verb());
        MarsRequest r      = lang.expand(*j, *j, inherit_, strict_);


        result.push_back(r);
    }

    return result;
}

MarsRequest MarsExpension::expand(const MarsRequest& request) {
    DummyContext ctx;
    MarsLanguage& lang = language(ctx, request.verb());
    return lang.expand(ctx, request, inherit_, strict_);
}


void MarsExpension::expand(const MarsExpandContext& ctx, const MarsRequest& request, ExpandCallback& callback) {
    MarsRequest r = language(ctx, request.verb()).expand(ctx, request, inherit_, strict_);
    callback(ctx, r);
}


void MarsExpension::flatten(const MarsExpandContext& ctx, const MarsRequest& request, FlattenCallback& callback) {
    language(ctx, request.verb()).flatten(ctx, request, callback);
}

//----------------------------------------------------------------------------------------------------------------------
}  // namespace mars
}  // namespace metkit
