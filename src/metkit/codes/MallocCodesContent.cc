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
#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/message/MessageContent.h"

#include "metkit/codes/CodesContent.h"
#include "metkit/codes/MallocCodesContent.h"

#include "eccodes.h"  /// @todo remove this depedency on eccodes fom here

namespace metkit {
namespace codes {

MallocCodesContent::MallocCodesContent(void* data, size_t size, const eckit::Offset& offset) :
    CodesContent(codes_handle_new_from_message(nullptr, data, size), true),
    buffer_(data),
    length_(size),
    offset_(offset) {}

MallocCodesContent::~MallocCodesContent() {
    ::free(buffer_);
}

void MallocCodesContent::print(std::ostream& s) const {
    s << "MallocCodesContent[]";
}

eckit::Offset MallocCodesContent::offset() const {
    return offset_;
}
const void* MallocCodesContent::data() const {
    return buffer_;
}
size_t MallocCodesContent::length() const {
    return length_;
}

}  // namespace codes
}  // namespace metkit
