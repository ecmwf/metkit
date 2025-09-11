/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Chris Bradley

#include <map>
#include <string>
#include <algorithm>
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Regex.h"
#include "eckit/utils/Tokenizer.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars/Matcher.h"

namespace metkit::mars {

namespace {  // helpers

using RegexMap = std::map<std::string, eckit::Regex>;

RegexMap parseKeyRegexList(const std::string& expr) {
    RegexMap out;
    if (expr.empty()) {
        return out;
    }

    std::vector<std::string> key_vals;
    eckit::Tokenizer(',')(expr, key_vals);

    eckit::Tokenizer equals('=');
    for (const std::string& item : key_vals) {
        std::vector<std::string> kv;
        equals(item, kv);
        if (kv.size() != 2) {
            throw eckit::UserError("Invalid condition " + item + " in expression: " + expr, Here());
        }

        const std::string& key = kv[0];
        const std::string& val = kv[1];

        if (out.find(key) != out.end()) {
            throw eckit::UserError("Duplicate key " + key + " in expression: " + expr, Here());
        }

        out[key] = eckit::Regex(val);
    }
    return out;
}

}  // namespace

Matcher::Matcher(const std::string& expr) : regexMap_{parseKeyRegexList(expr)} {}

bool Matcher::match(const MarsRequest& request, Policy policy, bool matchOnMissing) const {
    ValuesMap vals;
    for (const auto& kv : regexMap_) {
        vals[kv.first] = request.values(kv.first, /*emptyOK*/ true);
    }
    return match(vals, policy, matchOnMissing);
}

bool Matcher::match(const ValuesMap& request, Policy policy, bool matchOnMissing) const {
    return std::all_of(regexMap_.begin(), regexMap_.end(), [&](const auto& kv) {
        const auto& keyword = kv.first;
        const auto& re      = kv.second;

        auto it = request.find(keyword);

        // If the key is missing or present with no values
        if (it == request.end() || it->second.empty())
            return matchOnMissing;

        const auto& vals = it->second;
        auto pred        = [&](const std::string& s) { return re.match(s); };

        if (policy == Policy::Any)
            return std::any_of(vals.begin(), vals.end(), pred);
        else if (policy == Policy::All)
            return std::all_of(vals.begin(), vals.end(), pred);
        else
            throw eckit::SeriousBug("Not implemented ValuePolicy in Matcher::match");
    });
}

}  // namespace metkit::mars