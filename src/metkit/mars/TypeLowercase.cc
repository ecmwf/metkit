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

TypeLowercase::TypeLowercase(const std::string& type, Keyword key, const eckit::Value& val) : Type(type, key, val) {}

TypeLowercase::TypeLowercase(const std::string& type, Keyword key, MemFile& file) : Type(type, key, file) {}

void TypeLowercase::print(std::ostream& out) const {
    out << "TypeLowercase[name=" << name() << "]";
}

bool TypeLowercase::expand(std::string& value, const MarsRequest&) const {
    value = eckit::StringTools::lower(value);
    return true;
}


static TypeBuilder<TypeLowercase> type("lowercase");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
