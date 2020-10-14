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

#include "metkit/codes/DataContent.h"
// #include "metkit/codes/MessageContent.h"

#include "eckit/exception/Exceptions.h"
// #include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"
// #include "metkit/mars/MarsRequest.h"
// #include "metkit/codes/Decoder.h"

#include "eccodes.h"

namespace metkit {
namespace codes {

DataContent::DataContent(const void* data, size_t size):
    data_(data),
    size_(size),
    handle_(nullptr) {
}

DataContent::~DataContent() {
    if (handle_) {
        codes_handle_delete(handle_);
    }
}

eckit::DataHandle* DataContent::readHandle() const {
    return new eckit::MemoryHandle(data_, size_);
}

size_t DataContent::length() const {
    return size_;
}

const void* DataContent::data() const {
    return data_;
}

void DataContent::write(eckit::DataHandle& handle) const {
    if (handle.write(data_, size_) != size_) {
        std::ostringstream oss;
        oss << "Write error to data handle " << handle;
        throw eckit::WriteError(oss.str(), Here());
    }
}

const codes_handle* DataContent::codesHandle() const {
    if (!handle_) {
        handle_ = codes_handle_new_from_message(nullptr, data_, size_);
        ASSERT(handle_);
    }
    return handle_;
}


}  // namespace close
}  // namespace metkit

