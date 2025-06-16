/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars/TypeAny.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeAny::TypeAny(const std::string& name, const eckit::Value& settings) : Type(name, settings) {}

void TypeAny::print(std::ostream& out) const {
    out << "TypeAny[name=" << name_ << "]";
}

bool TypeAny::expand(const MarsExpandContext& /* ctx */, std::string& /* value */,
                     const MarsRequest& /* request */) const {
    return true;
}


static TypeBuilder<TypeAny> type("any");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace mars
}  // namespace metkit
