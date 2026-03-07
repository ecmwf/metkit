/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/CodesSplitter.h"

#include "eccodes.h"

#include "eckit/io/PeekHandle.h"
#include "eckit/log/Log.h"
#include "eckit/message/Message.h"

#include "metkit/codes/CodesDataContent.h"
#include "metkit/codes/GribHandle.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/CodesTypes.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

CodesSplitter::CodesSplitter(eckit::PeekHandle& handle) : Splitter(handle) {}

CodesSplitter::~CodesSplitter() {}

eckit::message::Message CodesSplitter::next() {
    eckit::Offset off = handle_.position();
    std::unique_ptr<CodesHandle> codesHandle;
    try {
        codesHandle =
            codesHandleFromStream([&](uint8_t* buffer, int64_t len) -> int64_t { return handle_.read(buffer, len); });
    }
    catch (const CodesWrongLength& e) {
        // METK-103 - used in bufr-sanity-check tool
        // Resetting handle and skipping header (ie. GRIB or BUFR)
        // Allows ignoring messages inbetween and continuing
        handle_.seek(off + eckit::Offset(4));
        throw e;
    }

    // Handle EOF
    if (!codesHandle) {
        return eckit::message::Message();
    }

    return eckit::message::Message(new CodesDataContent(std::move(codesHandle), off));
}

void CodesSplitter::print(std::ostream& s) const {
    s << "CodesSplitter[]";
}

}  // namespace codes
}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------

namespace eckit {
namespace message {

template <>
bool SplitterBuilder<metkit::codes::CodesSplitter>::match(eckit::PeekHandle& handle) const {

    unsigned char c0 = handle.peek(0);
    unsigned char c1 = handle.peek(1);
    unsigned char c2 = handle.peek(2);
    unsigned char c3 = handle.peek(3);

    if (c0 == 'G' and c1 == 'R' and c2 == 'I' and c3 == 'B') {
        return true;
    }

    if (c0 == 'B' and c1 == 'U' and c2 == 'F' and c3 == 'R') {
        return true;
    }

    if (c0 == 'B' and c1 == 'U' and c2 == 'D' and c3 == 'G') {
        return true;
    }

    if (c0 == 'T' and c1 == 'I' and c2 == 'D' and c3 == 'E') {
        return true;
    }

    return false;
}

}  // namespace message
}  // namespace eckit

static eckit::message::SplitterBuilder<metkit::codes::CodesSplitter> splitter;
