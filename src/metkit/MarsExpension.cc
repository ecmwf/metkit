/*
 * (C) Copyright 1996-2013 ECMWF.
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

using namespace eckit;

namespace metkit {

MarsExpension::MarsExpension() {

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

//----------------------------------------------------------------------------------------------------------------------

MarsLanguage& MarsExpension::language(const std::string& verb) {

    std::string v = MarsLanguage::expandVerb(verb);

    std::map<std::string, MarsLanguage*>::iterator j = languages_.find(v);
    if (j == languages_.end()) {
        languages_.insert(std::make_pair(v, new MarsLanguage(v)));
        j = languages_.find(v);
    }
    return *(*j).second;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<MarsRequest> MarsExpension::expand(const std::vector<MarsRequest>& requests) {
    std::vector<MarsRequest> result;

    // Implement inheritence
    for (std::vector<MarsRequest>::const_iterator j = requests.begin(); j != requests.end(); ++j) {

        MarsLanguage& lang = language((*j).name());
        MarsRequest r = lang.expand(*j);


        result.push_back(r);

    }

    return result;
}

//----------------------------------------------------------------------------------------------------------------------
void MarsExpension::expand(const MarsRequest& request, ExpandCallback& cb) {
    cb(language(request.name()).expand(request));
}

//----------------------------------------------------------------------------------------------------------------------


void MarsExpension::flatten(const MarsRequest& request,
                            FlattenCallback& callback,
                            FlattenFilter& filter) {
    MarsLanguage& lang = language(request.name());
    lang.flatten(request, callback, filter);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
