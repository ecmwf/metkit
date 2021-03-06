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


#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "eckit/io/PeekHandle.h"

#include "metkit/codes/MallocDataContent.h"
#include "metkit/codes/GribHandle.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

CodesSplitter::CodesSplitter(eckit::PeekHandle& handle):
    Splitter(handle) {}

CodesSplitter::~CodesSplitter() {}

static long readcb(void* data, void* buffer, long len) {
    eckit::DataHandle* handle = reinterpret_cast<eckit::DataHandle*>(data);
    return handle->read(buffer, len);
}

eckit::message::Message CodesSplitter::next() {
    size_t size;
    int err = 0;
    void *data = wmo_read_any_from_stream_malloc(&handle_, &readcb, &size, &err);

    if(err != 0 and err != GRIB_END_OF_FILE) {
        CODES_CALL(err);
    }
    
    if(!data) {
        return eckit::message::Message();
    }

    return eckit::message::Message(new MallocDataContent(data, size, 0));
}

void CodesSplitter::print(std::ostream& s) const {
    s << "CodesSplitter[]";
}

}  // namespace codes
}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------

namespace eckit {
namespace message {

template<>
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

