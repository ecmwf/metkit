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

#include <string>
#include <map>
#include "eckit/utils/Regex.h"
#include "metkit/mars/MarsRequest.h"
namespace metkit::mars {

// for handling things like:
//  expr: expver=(0001|o[0-9a-z]{3}),dataset=^climate-dt$
// the expr is a series of comma separated key=regex pairs
// where the regex is used to match against the value(s) of the key in a mars request


class Matcher {
public:
    enum class ValuePolicy { Any, All };
public:
    // mars key -> regex
    using RegexMap  = std::map<std::string, eckit::Regex>;

    using SelectMap = std::map<std::string, eckit::Regex>;
    using ValuesMap = std::map<std::string, std::vector<std::string>>;  // keyword -> list of values

public:
    Matcher(const std::string& expr);

    bool match(const ValuesMap& request, bool matchOnMissing, ValuePolicy policy) const;
    bool match(const MarsRequest& request, bool matchOnMissing, ValuePolicy policy) const;

    const RegexMap& regexMap() const { return regexMap_; }

private:
    
    RegexMap regexMap_; // public for now...
};

}  // namespace metkit::mars