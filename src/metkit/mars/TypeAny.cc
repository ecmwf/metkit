// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/mars/TypeAny.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypesFactory.h"

namespace {
const std::string typeName = "any";
}

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeAny::TypeAny(const std::string& type, Keyword key, const eckit::Value& val) : Type(type, key, val) {}
TypeAny::TypeAny(const std::string& type, Keyword key, MemFile& file) : Type(type, key, file) {}
TypeAny::TypeAny(const std::string& name) : Type(typeName, MarsLanguage::addKeyword(name), eckit::Value()) {}

void TypeAny::print(std::ostream& out) const {
    out << "TypeAny[name=" << name() << "]";
}

bool TypeAny::expand(std::string&, const MarsRequest&) const {
    return true;
}

static TypeBuilder<TypeAny> type("any");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
