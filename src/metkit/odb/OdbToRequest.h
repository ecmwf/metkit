/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_OdbToRequest_H
#define metkit_OdbToRequest_H

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

    bool one_                 = false;
    // bool mergeSimilarBlocks_  = false; // unused
    bool onlyConstantColumns_ = false;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace odb
}  // namespace metkit

#endif
