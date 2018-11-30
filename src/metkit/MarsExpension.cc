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
#include <set>
#include <list>

#include "eckit/types/Types.h"
#include "eckit/parser/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/config/Resource.h"
#include "eckit/utils/Translator.h"
#include "eckit/utils/MD5.h"
#include "eckit/parser/StringTools.h"

#include "metkit/MarsExpension.h"
#include "metkit/types/Type.h"
#include "metkit/MarsLanguage.h"



namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

FlattenCallback::~FlattenCallback() {}

ExpandCallback::~ExpandCallback() {}

//----------------------------------------------------------------------------------------------------------------------

MarsExpension::MarsExpension(bool inherit):
    inherit_(inherit) {

}

MarsExpension::~MarsExpension() {
    for (std::map<std::string, MarsLanguage* >::iterator j = languages_.begin(); j != languages_.end(); ++j) {
        delete (*j).second;
    }
}

void MarsExpension::reset() {
    for (std::map<std::string, MarsLanguage* >::iterator j = languages_.begin(); j != languages_.end(); ++j) {
        (*j).second->reset();
    }
}


MarsLanguage& MarsExpension::language(const std::string& verb, const MarsRequestContext& ctx) {

    std::string v = MarsLanguage::expandVerb(verb, ctx);

    std::map<std::string, MarsLanguage*>::iterator j = languages_.find(v);
    if (j == languages_.end()) {
        languages_.insert(std::make_pair(v, new MarsLanguage(v)));
        j = languages_.find(v);
    }
    return *(*j).second;
}


std::vector<MarsRequest> MarsExpension::expand(const std::vector<MarsRequest>& requests) {
    std::vector<MarsRequest> result;

    // Implement inheritence
    for (std::vector<MarsRequest>::const_iterator j = requests.begin(); j != requests.end(); ++j) {

        MarsLanguage& lang = language((*j).verb(), (*j).context());
        MarsRequest r = lang.expand(*j, inherit_);


        result.push_back(r);

    }

    return result;
}

void MarsExpension::expand(const MarsRequestContext& ctx, const MarsRequest& request, ExpandCallback& callback) {
    MarsRequest r = language(request.verb(), request.context()).expand(request, inherit_);
    callback(r);
}


void MarsExpension::flatten(const MarsRequest& request,
                            FlattenCallback& callback) {
    language(request.verb(), request.context()).flatten(request, callback);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
