/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */



#include <eccodes.h>

#include "metkit/codes/CodesSplitter.h"
#include "metkit/codes/CodesContent.h"
#include "metkit/codes/MallocDataContent.h"

#include "eckit/config/Resource.h"
#include "metkit/data/Message.h"
#include "metkit/mars/MarsRequest.h"
#include "eckit/io/PeekHandle.h"
#include "metkit/grib/GribHandle.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------
CodesSplitter::CodesSplitter(eckit::PeekHandle& handle):
    Splitter(handle) {
    file_ = handle.openf();
}

CodesSplitter::~CodesSplitter() {
    if (file_) {
        fclose(file_);
    }
}


data::Message CodesSplitter::next() {
    size_t size;
    off_t offset;
    int err = 0;
    void *data = wmo_read_any_from_file_malloc(file_, 0, &size, &offset, &err);

    CODES_CALL(err);
    ASSERT(err == 0);
    if(!data) {
        return data::Message();
    }
    return data::Message(new MallocDataContent(data, size));

}

void CodesSplitter::print(std::ostream& s) const {
    s << "CodesSplitter[]";
}

}  // namespace codes


template<>
bool data::SplitterBuilder<codes::CodesSplitter>::match(eckit::PeekHandle& handle) const {

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

static data::SplitterBuilder<codes::CodesSplitter> splitter;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
