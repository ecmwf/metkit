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

std::vector<std::vector<std::vector<double>>> GribJump::directJump(eckit::DataHandle* handle,
    std::vector<std::vector<std::tuple<size_t, size_t>>> allRanges,
    std::vector<JumpInfo> infos) {

    JumpHandle dataSource(handle);
    std::vector<std::vector<std::vector<double>>> result; 
    for (size_t i = 0; i < infos.size(); ++i) {
        JumpInfo info = infos[i];
        info.setStartOffset(dataSource.position());
        ASSERT(info.ready());
        std::vector<std::tuple<size_t, size_t>> ranges = allRanges[i];
        std::vector<double> v = info.extractRanges(dataSource, ranges);

        // extractRanges returns a flattened vector, so we need to unflatten it
        // TODO: avoid flattening in the first place
        std::vector<std::vector<double>> v2;
        size_t n = 0;
        for (auto range : ranges) {
            size_t len = std::get<1>(range) - std::get<0>(range);
            std::vector<double> v3(v.begin() + n, v.begin() + n + len);
            v2.push_back(v3);
            n += len;
        }
        result.push_back(v2);

        // Seek to start of next message
        dataSource.seek(info.length()); 
    }

    return result;
}

JumpInfo GribJump::extractInfo(eckit::DataHandle* handle) {
    JumpHandle dataSource(handle);
    return dataSource.extractInfo();
}

} // namespace GribJump
} // namespace metkit