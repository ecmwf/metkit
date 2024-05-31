/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Rule.h
/// @author Philipp Geier
/// @date   April 2024

#ifndef metkit_Rule_H
#define metkit_Rule_H

#include "metkit/mars2grib/KeySetter.h"
#include "eckit/value/Value.h"

namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

class GenericRule {
public:
    virtual ~GenericRule() {};
    virtual void apply(const eckit::ValueMap& inital, eckit::ValueMap& workDict, KeySetter& out) = 0;
};


using Rules = std::vector<std::unique_ptr<GenericRule>>;

class RuleList {
public:
    RuleList(Rules&&);

    void apply(const eckit::ValueMap& inital, KeySetter& out);

private:
    Rules rules_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib

#endif
