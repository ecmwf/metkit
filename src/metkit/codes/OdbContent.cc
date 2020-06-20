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


#include "metkit/codes/OdbContent.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"

namespace metkit {
namespace codes {

OdbContent::OdbContent(eckit::DataHandle& handle):
    frame_(handle.size()) {
    ASSERT(handle.read(frame_, frame_.size()) == frame_.size());
}

OdbContent::~OdbContent() {

}

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

void OdbContent::print(std::ostream & s) const {
    s << "OdbContent[]";
}


}  // namespace close
}  // namespace metkit

