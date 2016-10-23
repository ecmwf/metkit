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
#include "metkit/types/TypePostProc.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypePostProc::TypePostProc(const std::string &name, const eckit::Value& settings) :
    TypeFloat(name, settings) {
}

TypePostProc::~TypePostProc() {
}

void TypePostProc::print(std::ostream &out) const {
    out << "TypePostProc[name=" << name_ << "]";
}


void TypePostProc::flattenValues(const MarsRequest& request, std::vector<std::string>& values) {
    // Empty, so that the values are not considered a list when flattening the requests
}

static TypeBuilder<TypePostProc> type("postproc");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
