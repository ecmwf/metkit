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
    ctx_.clear();
}

std::vector<MarsRequest> MarsExpansion::expand(const std::vector<MarsParsedRequest>& requests) {
    std::vector<MarsRequest> result;
    result.reserve(requests.size());

    // Implement inheritence
    for (const auto& request : requests) {
        auto verb = MarsLanguage::expandVerb(request.verb());
        result.emplace_back(MarsLanguage::get(verb).expand(request, ctxForVerb(verb), inherit_, strict_));
    }

    return result;
}

std::vector<MarsRequest> MarsExpansion::expand(const std::vector<MarsRequest>& requests) {
    std::vector<MarsRequest> result;
    result.reserve(requests.size());

    for (const auto& request : requests) {
        auto verb = MarsLanguage::expandVerb(request.verb());
        result.emplace_back(MarsLanguage::get(verb).expand(request, ctxForVerb(verb), inherit_, strict_));
    }

    return result;
}

MarsRequest MarsExpansion::expand(const MarsRequest& request) {
    auto verb = MarsLanguage::expandVerb(request.verb());
    return MarsLanguage::get(verb).expand(request, ctxForVerb(verb), inherit_, strict_);
}

void MarsExpansion::expand(const MarsRequest& request, ExpandCallback& callback) {
    callback(expand(request));
}

void MarsExpansion::flatten(const MarsRequest& request, FlattenCallback& callback) {
    auto verb = MarsLanguage::expandVerb(request.verb());
    MarsLanguage::get(verb).flatten(request, callback);
}

ExpansionContext& MarsExpansion::ctxForVerb(const std::string& verb) {
    static ExpansionContext dummy;
    if (!inherit_) {
        return dummy;
    }
    auto it = ctx_.find(verb);
    if (it == ctx_.end()) {
        it = ctx_.emplace(verb, new ExpansionContext{}).first;
    }
    return *(it->second);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
