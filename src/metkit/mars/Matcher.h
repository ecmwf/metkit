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
#include "eckit/utils/Regex.h"
#include "metkit/mars/MarsRequest.h"
namespace metkit::mars {

class Matcher {

    using RegexMap = std::map<std::string, eckit::Regex>;
    // keyword -> list of values
    using ValuesMap = std::map<std::string, std::vector<std::string>>;

public:

    // mars key -> regex
    enum class Policy {
        Any,
        All
    };

public:

    // expr is a series of comma separated key=regex pairs e.g.,
    //     "expver=(0001|o[0-9a-z]{3}),dataset=^climate-dt$"
    Matcher(const std::string& expr);

    bool match(const ValuesMap& request, Policy policy, bool matchOnMissing = true) const;
    bool match(const MarsRequest& request, Policy policy, bool matchOnMissing = true) const;

    const RegexMap& regexMap() const { return regexMap_; }

private:

    RegexMap regexMap_;
};

}  // namespace metkit::mars