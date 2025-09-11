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

#include <map>
#include <string>
#include "eckit/utils/Regex.h"
#include "metkit/mars/MarsRequest.h"
namespace metkit::mars {

class RequestAccessor;

/// Matcher class to match requests against a set of key=regex conditions,
/// for constructing select/exclude filters.
class Matcher {

public:

    enum class Policy {
        All,  ///< Require all values to match
        Any   ///< Require at least one value to match
    };

public:

    /// @param expr is a series of comma separated key=regex pairs e.g.
    ///   "expver=(0001|o[0-9a-z]{3}),dataset=^climate-dt$"
    /// @param policy defines how we treat requests with multiple values for a given key.
    Matcher(const std::string& expr, Policy policy);

    /// @param matchOnMissing: if true, then keys in the matcher absent in the request match.
    bool match(const RequestAccessor& request, bool matchOnMissing = true) const;
    bool match(const MarsRequest& request, bool matchOnMissing = true) const;

private:

    const std::map<std::string, eckit::Regex> regexMap_;
    const Policy policy_;
};

// ----------------------------------------------------------------------------------------------------------------------

// Wrapper around a request (e.g., MarsRequest, fdb5::Key, etc) to provide access to its values
class RequestAccessor {
public:

    using values_t = std::variant<std::reference_wrapper<const std::string>,              // single value
                                  std::reference_wrapper<const std::vector<std::string>>  // multiple values
                                  >;

    virtual ~RequestAccessor() = default;

    // Returns true if the keyword exists in the request
    virtual bool has(const std::string& keyword) const = 0;

    // Get a reference_wrapper to the value(s) for the given keyword
    virtual values_t get(const std::string& keyword) const = 0;
};

}  // namespace metkit::mars