// SPDX-FileCopyrightText: 1996-2012 European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <map>
#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Length.h"
#include "eckit/io/Offset.h"

#include "metkit/mars/MarsRequest.h"

namespace eckit {
class DataHandle;
}

namespace metkit {
using namespace mars;

namespace odb {

//----------------------------------------------------------------------------------------------------------------------

class OdbToRequest {
public:  // methods

    OdbToRequest(const std::string& verb, bool one, bool constant);
    ~OdbToRequest();

    std::vector<MarsRequest> odbToRequest(eckit::DataHandle& dh) const;

private:  // members

    std::string verb_ = "retrieve";

    bool one_ = false;
    // bool mergeSimilarBlocks_  = false; // unused
    bool onlyConstantColumns_ = false;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace odb
}  // namespace metkit
