/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */




#include "metkit/codes/OdbSplitter.h"

#include "eckit/config/Resource.h"
#include "metkit/codes/Message.h"
#include "metkit/mars/MarsRequest.h"
#include "eckit/io/PeekHandle.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------
OdbSplitter::OdbSplitter(eckit::DataHandle& handle):
    Splitter(handle) {
}

OdbSplitter::~OdbSplitter() {
}


Message OdbSplitter::next() {
    return Message();
}

void OdbSplitter::print(std::ostream& s) const {
    s << "OdbSplitter[]";
}

template<>
bool SplitterBuilder<OdbSplitter>::match(eckit::PeekHandle& handle) const {

    unsigned char c0 = handle.peek(0);
    unsigned char c1 = handle.peek(1);
    unsigned char c2 = handle.peek(2);
    unsigned char c3 = handle.peek(3);
    unsigned char c4 = handle.peek(4);

    if (c0 == 0xff and c1 == 0xff and c2 == 'O' and c3 == 'D' and c4 == 'A') {
        return true;
    }

    return false;
}

static SplitterBuilder<OdbSplitter> splitter;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
