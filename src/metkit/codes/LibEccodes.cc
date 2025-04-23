/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Olivier Iffrig
/// @date   October 2019

#include "eccodes.h"

#include "metkit/codes/LibEccodes.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

REGISTER_LIBRARY(LibEccodes);

LibEccodes::LibEccodes() : Library("eccodes") {}

const LibEccodes& LibEccodes::instance() {
    static LibEccodes libeccodes;
    return libeccodes;
}

std::string LibEccodes::version() const {
    return ECCODES_VERSION_STR;
}

std::string LibEccodes::gitsha1(unsigned int count) const {
    std::string sha1(codes_get_git_sha1());
    if (sha1.empty()) {
        return "not available";
    }

    return sha1.substr(0, std::min(count, 40u));
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
