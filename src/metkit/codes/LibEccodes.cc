// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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
