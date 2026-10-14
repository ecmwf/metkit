// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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
    return ctx_[verb];
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
