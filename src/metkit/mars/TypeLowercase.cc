/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars/TypeLowercase.h"
#include "metkit/mars/TypesFactory.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeLowercase::TypeLowercase(const std::string& name, const eckit::Value& settings) : Type(name, settings) {}

void TypeLowercase::print(std::ostream& out) const {
    out << "TypeLowercase[name=" << name_ << "]";
}

bool TypeLowercase::expand(const MarsExpandContext& ctx, std::string& value, const MarsRequest& /* request */) const {

    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
    return true;
}


static TypeBuilder<TypeLowercase> type("lowercase");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
