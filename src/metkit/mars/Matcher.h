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
/// @date Sep 2025

#pragma once

#include <functional>
#include <map>
#include <string>
#include <variant>
#include "eckit/utils/Regex.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

class RequestLike;

/// Parse a match expression into a map of key:regex pairs
/// @param expr is a series of comma separated key:regex pairs e.g.
///   "expver=(0001|o[0-9a-z]{3}),dataset=^climate-dt$"
std::map<std::string, eckit::Regex> parseMatchString(const std::string& expr);

// -------------------------------------------------------------------------------------------------------------

/// For matching requests against a set of key:regex conditions. For constructing select/exclude filters.

class Matcher {

public:

    /// Policy for handling requests with multiple values for a given key.
    enum class Policy {
        All,  ///< Require all values to match
        Any   ///< Require at least one value to match
    };

    /// Policy for handling keys in the matcher that are absent in the request.
    enum class MatchMissingPolicy {
        MatchOnMissing,
        DontMatchOnMissing
    };
    static constexpr auto MatchOnMissing     = MatchMissingPolicy::MatchOnMissing;
    static constexpr auto DontMatchOnMissing = MatchMissingPolicy::DontMatchOnMissing;


public:

    /// Construct a matcher from a map of key:regex pairs
    Matcher(std::map<std::string, eckit::Regex> regexMap, Policy policy);

    Matcher(const std::string& expr, Policy policy);

    bool match(const RequestLike& request, MatchMissingPolicy matchOnMissing = MatchOnMissing) const;
    bool match(const MarsRequest& request, MatchMissingPolicy matchOnMissing = MatchOnMissing) const;

private:

    std::map<std::string, eckit::Regex> regexMap_;
    Policy policy_;
};

// ----------------------------------------------------------------------------------------------------------------------

/// Interface to access keyword / value(s) pairs across similar types.
/// Designed to allow uniform access to keywords / values for MarsRequest / fdb5::Key
class RequestLike {
public:

    using values_t = std::variant<std::reference_wrapper<const std::string>,              ///< single value
                                  std::reference_wrapper<const std::vector<std::string>>  ///< multiple values
                                  >;

    virtual ~RequestLike() = default;

    /// Get a reference_wrapper to the value(s) for the given keyword, or nullopt if not found
    virtual std::optional<values_t> get(const std::string& keyword) const = 0;
};

}  // namespace metkit::mars