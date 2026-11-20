// SPDX-FileCopyrightText: 2017- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @date   Jun 2020


#include "metkit/odb/OdbContent.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"

namespace metkit::codes {

OdbContent::OdbContent(eckit::Buffer&& buffer) : frame_(std::move(buffer)) {}

OdbContent::OdbContent(eckit::DataHandle& handle, size_t size) : frame_(size) {
    ASSERT(handle.read(frame_, frame_.size()) == frame_.size());
}

OdbContent::~OdbContent() {}

const void* OdbContent::data() const {
    return frame_;
}

size_t OdbContent::length() const {
    return frame_.size();
}

eckit::DataHandle* OdbContent::readHandle() const {
    return new eckit::MemoryHandle(frame_, frame_.size());
}

void OdbContent::write(eckit::DataHandle& handle) const {
    ASSERT(handle.write(frame_, frame_.size()) == frame_.size());
}

void OdbContent::print(std::ostream& s) const {
    s << "OdbContent[]";
}


}  // namespace metkit::codes
