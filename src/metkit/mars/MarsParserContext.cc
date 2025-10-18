/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/MarsParserContext.h"

#include <iostream>

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------
MarsParserContext::MarsParserContext(std::size_t line) :
    line_(line)
{}

void MarsParserContext::info(std::ostream& out) const {
    out << " Request starting line " << line_;
}

}  // namespace metkit::mars
