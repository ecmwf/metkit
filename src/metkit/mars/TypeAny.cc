// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/mars/TypeAny.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeAny::TypeAny(const std::string& name, const eckit::Value& settings) : Type(name, settings) {}

void TypeAny::print(std::ostream& out) const {
    out << "TypeAny[name=" << name_ << "]";
}

bool TypeAny::expand(std::string&, const MarsRequest&) const {
    return true;
}

static TypeBuilder<TypeAny> type("any");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
