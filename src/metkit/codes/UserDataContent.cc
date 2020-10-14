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

#include "metkit/codes/UserDataContent.h"


namespace metkit {
namespace codes {

UserDataContent::UserDataContent(const void* data, size_t size):
    DataContent(data, size) {
}

UserDataContent::~UserDataContent() {
}

void UserDataContent::print(std::ostream & s) const {
    s << "UserDataContent[]";
}

}  // namespace close
}  // namespace metkit

