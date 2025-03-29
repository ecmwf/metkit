/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <iomanip>

#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/TypeExpver.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeExpver::TypeExpver(const std::string& name, const eckit::Value& settings) : Type(name, settings) {}

TypeExpver::~TypeExpver() {}

bool TypeExpver::expand(const MarsExpandContext&, std::string& value) const {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << eckit::StringTools::trim(value);
    value = oss.str();
    return true;
}

void TypeExpver::print(std::ostream& out) const {
    out << "TypeExpver[name=" << name_ << "]";
}

static TypeBuilder<TypeExpver> type("expver");

//----------------------------------------------------------------------------------------------------------------------
}  // namespace mars
}  // namespace metkit
