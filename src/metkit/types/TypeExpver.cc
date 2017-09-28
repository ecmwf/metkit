/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/utils/Translator.h"

#include "eckit/types/Date.h"
#include "metkit/MarsRequest.h"

#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeExpver.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeExpver::TypeExpver(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {
}

TypeExpver::~TypeExpver() {
}

bool TypeExpver::expand(std::string& value) const {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << value;
    value = oss.str();
    return true;
}

void TypeExpver::print(std::ostream &out) const {
    out << "TypeExpver[name=" << name_ << "]";
}

static TypeBuilder<TypeExpver> type("expver");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
