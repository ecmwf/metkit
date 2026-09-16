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

MarsExpansion::MarsExpansion(bool inherit, bool strict) : inherit_(inherit), strict_(strict) {}

void MarsExpansion::reset() {
    for (auto& [verb, ctx] : contextes_) {
        ctx = MarsRequest{};
    }
}

const MarsLanguage& MarsExpansion::language(const std::string& verb) const {
    auto v = MarsLanguage::expandVerb(verb);

    return MarsLanguageRegistry::instance().language(v);

    // if (auto j = languages_.find(v); j != languages_.end()) {
    //     return *(*j).second;
    // }


    // auto j = languages_.emplace(v, &(MarsLanguageRegistry::instance().language(v))).first;
    // return *(*j).second;
}


std::vector<MarsRequest> MarsExpansion::expand(const std::vector<MarsParsedRequest>& requests) {
    MarsRequest ctx;
    std::vector<MarsRequest> result;
    result.reserve(requests.size());

    // Implement inheritence
    for (const auto& request : requests) {
        auto& lang = language(request.verb());
        if (!inherit_) {
            ctx = MarsRequest{};
        }
        result.emplace_back(lang.expand(request, ctx, inherit_, strict_));
    }

    return result;
}

std::vector<MarsRequest> MarsExpansion::expand(const std::vector<MarsRequest>& requests) {
    MarsRequest ctx;
    std::vector<MarsRequest> result;
    result.reserve(requests.size());

    for (const auto& request : requests) {
        auto& lang = language(request.verb());
        if (!inherit_) {
            ctx = MarsRequest{};
        }
        result.emplace_back(lang.expand(request, ctx, inherit_, strict_));
    }

    return result;
}

MarsRequest MarsExpansion::expand(const MarsRequest& request) {
    MarsRequest ctx;
    return language(request.verb()).expand(request, ctx, inherit_, strict_);
}

void MarsExpansion::expand(const MarsRequest& request, ExpandCallback& callback) {
    callback(expand(request));
}

void MarsExpansion::flatten(const MarsRequest& request, FlattenCallback& callback) {
    language(request.verb()).flatten(request, callback);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
