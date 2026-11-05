// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Olivier Iffrig
/// @date   October 2019

#pragma once

#include "eckit/system/Library.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class LibEccodes : public eckit::system::Library {
public:

    LibEccodes();

    static const LibEccodes& instance();

protected:

    std::string version() const override;

    std::string gitsha1(unsigned int count) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
