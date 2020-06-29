/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jun 2020

#include <iostream>

#include "metkit/codes/MallocDataContent.h"


namespace metkit {
namespace codes {

MallocDataContent::MallocDataContent(void* data, size_t size):
    DataContent(data, size),
    buffer_(data) {
}

MallocDataContent::~MallocDataContent() {
    ::free(buffer_);
}

void MallocDataContent::print(std::ostream & s) const {
    s << "MallocDataContent[]";
}


}  // namespace close
}  // namespace metkit

