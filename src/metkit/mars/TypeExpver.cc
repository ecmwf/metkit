// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <iomanip>

#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "metkit/mars/TypeExpver.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeExpver::TypeExpver(const std::string& type, MarsKeyword key, const eckit::Value& value) : Type(type, key, value) {}
TypeExpver::TypeExpver(const std::string& type, MarsKeyword key, MMapFileReader& file) : Type(type, key, file) {}

bool TypeExpver::expand(std::string& value, const MarsRequest&) const {

    std::string v = eckit::StringTools::lower(eckit::StringTools::trim(value));

    if (v == "all" || v == "any") {
        value = v;
    }
    else {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << eckit::StringTools::trim(value);
        value = oss.str();
    }
    return true;
}

void TypeExpver::print(std::ostream& out) const {
    out << "TypeExpver[name=" << name() << "]";
}

static TypeBuilder<TypeExpver> type("expver");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
