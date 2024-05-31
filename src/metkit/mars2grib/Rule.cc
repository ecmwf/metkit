/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars2grib/Rule.h"


namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------


RuleList::RuleList(Rules&& rules) :
    rules_{std::move(rules)} {};

void RuleList::apply(const eckit::ValueMap& initial, KeySetter& out) {
    eckit::ValueMap workdict{initial};

    for (auto& rule : rules_) {
        rule->apply(initial, workdict, out);
    }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib
