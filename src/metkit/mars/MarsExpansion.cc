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


MarsLanguage& MarsExpansion::language(const MarsExpandContext& ctx, const std::string& verb) {
    auto v = MarsLanguage::expandVerb(ctx, verb);

    if (auto j = languages_.find(v); j != languages_.end()) {
        return *(*j).second;
    }

    auto j = languages_.emplace(v, new MarsLanguage(v)).first;
    return *(*j).second;
}


std::vector<MarsRequest> MarsExpansion::expand(const std::vector<MarsParsedRequest>& requests) {
    std::vector<MarsRequest> result;
    result.reserve(requests.size());

    DummyContext cc;
    MarsExpandContext& ctx = cc;

    // Implement inheritence
    for (size_t i = 0; i < requests.size(); i++) {
        auto& lang = language(ctx, requests.at(i).verb());
        result.emplace_back(lang.expand(ctx, requests.at(i), inherit_, strict_));
        ctx = requests.at(i);
    }

    return result;
}

MarsRequest MarsExpansion::expand(const MarsRequest& request) {
    DummyContext ctx;
    auto& lang = language(ctx, request.verb());
    return lang.expand(ctx, request, inherit_, strict_);
}


void MarsExpansion::expand(const MarsExpandContext& ctx, const MarsRequest& request, ExpandCallback& callback) {
    MarsRequest r = language(ctx, request.verb()).expand(ctx, request, inherit_, strict_);
    callback(ctx, r);
}


void MarsExpansion::flatten(const MarsExpandContext& ctx, const MarsRequest& request, FlattenCallback& callback) {
    language(ctx, request.verb()).flatten(ctx, request, callback);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
