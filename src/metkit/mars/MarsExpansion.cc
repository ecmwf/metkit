/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/MarsExpansion.h"

#include "metkit/mars/MarsLanguage.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

FlattenCallback::~FlattenCallback() = default;

ExpandCallback::~ExpandCallback() = default;

//----------------------------------------------------------------------------------------------------------------------

MarsExpansion::MarsExpansion(bool inherit, bool strict) : inherit_(inherit), strict_(strict) {}

MarsExpansion::~MarsExpansion() {
    for (auto& language : languages_) {
        delete language.second;
    }
}

void MarsExpansion::reset() {
    for (auto& language : languages_) {
        language.second->reset();
    }
}


MarsLanguage& MarsExpansion::language(const std::string& verb) {
    auto v = MarsLanguage::expandVerb(verb);

    if (auto j = languages_.find(v); j != languages_.end()) {
        return *(*j).second;
    }

    auto j = languages_.emplace(v, new MarsLanguage(v)).first;
    return *(*j).second;
}


std::vector<MarsRequest> MarsExpansion::expand(const std::vector<MarsParsedRequest>& requests) {
    std::vector<MarsRequest> result;
    result.reserve(requests.size());

    // Implement inheritence
    for (const auto& request : requests) {
        auto& lang = language(request.verb());
        result.emplace_back(lang.expand(request, inherit_, strict_));
    }

    return result;
}

MarsRequest MarsExpansion::expand(const MarsRequest& request) {
    auto& lang = language(request.verb());
    return lang.expand(request, inherit_, strict_);
}


void MarsExpansion::expand(const MarsRequest& request, ExpandCallback& callback) {
    MarsRequest r = language(request.verb()).expand(request, inherit_, strict_);
    callback(r);
}


void MarsExpansion::flatten(const MarsRequest& request, FlattenCallback& callback) {
    language(request.verb()).flatten(request, callback);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
