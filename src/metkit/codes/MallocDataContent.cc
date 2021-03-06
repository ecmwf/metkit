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

#include "eckit/message/MessageContent.h"
#include "eckit/exception/Exceptions.h"

#include "metkit/codes/MallocDataContent.h"
#include "metkit/codes/CodesContent.h"

#include "eccodes.h" /// @todo remove this depedency on eccodes fom here

namespace metkit {
namespace codes {

MallocDataContent::MallocDataContent(void* data, size_t size, const eckit::Offset& offset):
    DataContent(data, size),
    buffer_(data),
    offset_(offset) {
}

MallocDataContent::~MallocDataContent() {
    ::free(buffer_);
}

void MallocDataContent::print(std::ostream & s) const {
    s << "MallocDataContent[]";
}

eckit::Offset MallocDataContent::offset() const {
    return offset_;
}

eckit::message::MessageContent* MallocDataContent::transform(const eckit::StringDict& dict) const {

    /// This is a HACK !!!!
    /// TODO come back and work on the Transformers of Messages with proper double-dispatch

    codes_handle* h = codes_handle_new_from_message(nullptr, data(), length());
    if(!h) {
        throw eckit::FailedLibraryCall("eccodes", "codes_handle_new_from_message", "failed to create handle", Here());
    }

    std::unique_ptr<eckit::message::MessageContent> content(new metkit::codes::CodesContent(h, true));

    return content->transform(dict);
}


}  // namespace close
}  // namespace metkit

