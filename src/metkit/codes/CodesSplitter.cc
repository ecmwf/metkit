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

#include "eckit/config/Resource.h"
#include "metkit/codes/Message.h"
#include "metkit/mars/MarsRequest.h"
#include "eckit/io/PeekHandle.h"

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


Message CodesSplitter::next() {
    int err = 0;
    codes_handle* h = codes_handle_new_from_file(nullptr,
                      file_,
                      PRODUCT_ANY,
                      &err);
    ASSERT(err == 0);
    return Message(new CodesContent(h, true));
}

void CodesSplitter::print(std::ostream& s) const {
    s << "CodesSplitter[]";
}

template<>
bool SplitterBuilder<CodesSplitter>::match(eckit::PeekHandle& handle) const {

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

static SplitterBuilder<CodesSplitter> splitter;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
