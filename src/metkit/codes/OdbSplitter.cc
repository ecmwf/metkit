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

#include "eckit/message/Message.h"
//#include "eckit/io/PeekHandle.h"
#include "eckit/io/SeekableHandle.h"

#include "metkit/codes/OdbContent.h"
#include "metkit/codes/OdbMetadataDecoder.h"

#include "odc/api/Odb.h"

namespace metkit {
namespace codes {


OdbSplitter::OdbSplitter(eckit::PeekHandle& handle) : Splitter(handle) {
    handle.openForRead();
}

OdbSplitter::~OdbSplitter() {}

eckit::message::Message OdbSplitter::next() {

    eckit::SeekableHandle seekHandle{handle_};
    seekHandle.seek(handle_.position());

    eckit::Length length = 0;

    odc::api::Reader reader(seekHandle, false);
    odc::api::Frame frame = reader.next();
    if (frame) {
        odc::api::Span last = frame.span(OdbMetadataDecoder::columnNames(), true);
        length = frame.length();

        while ((frame = reader.next())) {
            odc::api::Span span = frame.span(OdbMetadataDecoder::columnNames(), true);

            if (span == last) {
                length += frame.length();
            } else {
                return eckit::message::Message{new OdbContent(handle_, length)};
            }
        }
    }
    if (length == eckit::Length(0)) {
        return eckit::message::Message();
    }

    return eckit::message::Message{new OdbContent(handle_, length)};
}

void OdbSplitter::print(std::ostream& s) const {
    s << "OdbSplitter[]";
}

}  // namespace codes
}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------

namespace eckit {
namespace message {

template <>
bool SplitterBuilder<metkit::codes::OdbSplitter>::match(
    eckit::PeekHandle& handle) const {
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

}  // namespace message
}  // namespace eckit

static eckit::message::SplitterBuilder<metkit::codes::OdbSplitter> splitter;
