/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/parser/StringTools.h"
#include "metkit/MarsRequest.h"

#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeArea.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeArea::TypeArea(const std::string &name, const eckit::Value& value) :
    Type(name, value) {
}

TypeArea::~TypeArea() {
}

void TypeArea::print(std::ostream &out) const {
    out << "TypeArea[name=" << name_ << "]";
}

static TypeBuilder<TypeArea> type("area");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
