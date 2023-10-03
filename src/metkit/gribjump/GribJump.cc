/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/gribjump/GribHandleData.h"
#include "metkit/gribjump/GribJump.h"

namespace metkit {
namespace gribjump {

GribJump::GribJump() {}

GribJump::~GribJump() {}

std::vector<std::vector<double>> GribJump::directJump(eckit::DataHandle* handle,
    std::vector<std::tuple<size_t, size_t>> ranges,
    JumpInfo info) const {
    JumpHandle dataSource(handle);
    info.setStartOffset(0); // Message starts at the beginning of the handle
    ASSERT(info.ready());
    return info.extractRanges(dataSource, ranges);
}

JumpInfo GribJump::extractInfo(eckit::DataHandle* handle) const {
    JumpHandle dataSource(handle);
    return dataSource.extractInfo();
}

} // namespace GribJump
} // namespace metkit