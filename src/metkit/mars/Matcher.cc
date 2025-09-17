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
#include <functional>
#include <map>
#include <string>
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Regex.h"
#include "eckit/utils/Tokenizer.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars/Matcher.h"

// ----------------------------------------------------------------------------------------------------------------------
namespace {
    
// Visitor pattern for std::variant
template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

} //

// ----------------------------------------------------------------------------------------------------------------------
namespace metkit::mars {
namespace {  // helpers
class MarsRequestAccessor : public RequestLike {
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

RegexMap parseMatchString(const std::string& expr) {
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
            throw eckit::BadValue("Invalid condition " + item + " in expression: " + expr, Here());
        }

        const std::string& key = kv[0];
        const std::string& val = kv[1];

        if (out.find(key) != out.end()) {
            throw eckit::BadValue("Duplicate key " + key + " in expression: " + expr, Here());
        }

        out[key] = eckit::Regex(val);
    }
    return out;
}

// ----------------------------------------------------------------------------------------------------------------------

Matcher::Matcher(std::map<std::string, eckit::Regex> regexMap, Policy policy) : regexMap_{std::move(regexMap)}, policy_{policy} {}

Matcher::Matcher(const std::string& expr, Policy policy) : regexMap_{parseMatchString(expr)}, policy_{policy} {}

bool Matcher::match(const MarsRequest& request, MatchMissingPolicy matchOnMissing) const {
    return match(MarsRequestAccessor(request), matchOnMissing);
}

bool Matcher::match(const RequestLike& request, MatchMissingPolicy matchOnMissing) const {
    return std::all_of(regexMap_.begin(), regexMap_.end(), [&](const auto& kv) {
        const auto& keyword = kv.first;
        const auto& regex      = kv.second;

        if (!request.has(keyword))
            return matchOnMissing == MatchOnMissing;

        auto pred = [&regex](const std::string& s) { return regex.match(s); };

        // clang-format off
        return std::visit(Overloaded {
            [&](std::reference_wrapper<const std::string> value) {
                return pred(value.get());
            },
            [&](std::reference_wrapper<const std::vector<std::string>> values) {
                const auto& vec = values.get();
                if (policy_ == Policy::Any) {
                    return std::any_of(vec.begin(), vec.end(), pred);
                }
                ASSERT(policy_ == Policy::All);
                return std::all_of(vec.begin(), vec.end(), pred);
            }
        }, request.get(keyword));
        // clang-format on
    });
}

}  // namespace metkit::mars