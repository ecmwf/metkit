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
// #include "metkit/codes/MessageContent.h"

#include "eckit/exception/Exceptions.h"
// #include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"
// #include "metkit/mars/MarsRequest.h"
// #include "metkit/codes/Decoder.h"

#include <eccodes.h>

namespace metkit {
namespace codes {

UserDataContent::UserDataContent(const void* data, size_t size):
    data_(data),
    size_(size),
    handle_(0) {
}

UserDataContent::~UserDataContent() {
    if (handle_) {
        codes_handle_delete(handle_);
    }
}

void UserDataContent::print(std::ostream & s) const {
    s << "UserDataContent[]";
}

eckit::DataHandle* UserDataContent::readHandle() const {
    return new eckit::MemoryHandle(data_, size_);
}

size_t UserDataContent::length() const {
    return size_;
}

const void* UserDataContent::data() const {
    return data_;
}

void UserDataContent::write(eckit::DataHandle& handle) const {
    ASSERT(handle.write(data_, size_) == size_);
}

const codes_handle* UserDataContent::codesHandle() const {
    if (!handle_) {
        handle_ = codes_handle_new_from_message(0, data_, size_);
        ASSERT(handle_);
    }
    return handle_;
}


}  // namespace close
}  // namespace metkit

