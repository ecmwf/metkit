// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "eckit/utils/StringTools.h"

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

bool TypeLowercase::expand(std::string& value, const MarsRequest&) const {
    value = eckit::StringTools::lower(value);
    return true;
}


static TypeBuilder<TypeLowercase> type("lowercase");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
