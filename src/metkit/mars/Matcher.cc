/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Chris Bradley

#include <algorithm>
#include <map>
#include <string>
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Regex.h"
#include "eckit/utils/Tokenizer.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars/Matcher.h"

// ----------------------------------------------------------------------------------------------------------------------
// Visitor pattern for std::variant
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// ----------------------------------------------------------------------------------------------------------------------
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

class MarsRequestAccessor : public RequestAccessor {
public:

    explicit MarsRequestAccessor(const metkit::mars::MarsRequest& request) : request_(request) {}

    bool has(const std::string& keyword) const override { return request_.has(keyword); }

    values_t get(const std::string& keyword) const override {
        // returns reference_wrapper<vector<string>>
        return std::cref(request_.values(keyword));
    }

private:

    const metkit::mars::MarsRequest& request_;
};

}  // namespace
// ----------------------------------------------------------------------------------------------------------------------

Matcher::Matcher(const std::string& expr, Policy policy) : regexMap_{parseKeyRegexList(expr)}, policy_{policy} {}

bool Matcher::match(const MarsRequest& request, bool matchOnMissing) const {
    return match(MarsRequestAccessor(request), matchOnMissing);
}

bool Matcher::match(const RequestAccessor& request, bool matchOnMissing) const {
    return std::all_of(regexMap_.begin(), regexMap_.end(), [&](const auto& kv) {
        const auto& keyword = kv.first;
        const auto& re      = kv.second;

        if (!request.has(keyword))
            return matchOnMissing;

        auto pred = [&](const std::string& s) { return re.match(s); };

        return std::visit(overloaded{[&](std::reference_wrapper<const std::string> value) { return pred(value.get()); },
                                     [&](std::reference_wrapper<const std::vector<std::string>> values) {
                                         const auto& vec = values.get();
                                         if (policy_ == Policy::Any) {
                                             return std::any_of(vec.begin(), vec.end(), pred);
                                         }
                                         else {
                                             ASSERT(policy_ == Policy::All);
                                             return std::all_of(vec.begin(), vec.end(), pred);
                                         }
                                     }},
                          request.get(keyword));
    });
}

}  // namespace metkit::mars