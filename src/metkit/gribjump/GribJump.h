/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_GribJump_H
#define metkit_GribJump_H

#include "metkit/gribjump/GribInfo.h"
#include "eckit/io/DataHandle.h"


namespace metkit {
namespace gribjump {

// Gribjump API
class GribJump : public eckit::NonCopyable {

public: 

    GribJump();
    ~GribJump();

    std::vector<std::vector<double>> directJump(eckit::DataHandle* handle, 
        std::vector<std::tuple<size_t, size_t>> allRanges,
        JumpInfo info) const;

    JumpInfo extractInfo(eckit::DataHandle* handle) const;

    bool isCached(std::string) const {return false;} // todo implement caching

private:

    // std::map<Key, std::tuple<FieldLocation*, JumpInfo> > cache_; // not imp
};


} // namespace GribJump
} // namespace metkit
#endif // fdb5_gribjump_LibGribJump_H