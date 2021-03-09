/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/odb/OdbSplitter.h"

#include "eckit/message/Message.h"
#include "eckit/io/PeekHandle.h"
#include "eckit/io/MemoryHandle.h"

#include "metkit/odb/OdbContent.h"
#include "metkit/odb/OdbMetadataDecoder.h"


namespace metkit {
namespace codes {

OdbSplitter::OdbSplitter(eckit::PeekHandle& handle) : Splitter(handle), eof_(false) {
    handle.openForRead();
}

OdbSplitter::~OdbSplitter() {}

eckit::message::Message OdbSplitter::next() {

    if (eof_) {
        return eckit::message::Message();
    }

    odc::api::Reader reader(handle_, false);    
    odc::api::Frame frame = reader.next();
    if (!frame) {
        eof_ = true;
        return eckit::message::Message();
    }

    odc::api::Span last = frame.span(OdbMetadataDecoder::columnNames(), true);
    eckit::Buffer buffer = frame.encodedData();
    eckit::Length offset = frame.length();

    while ((frame = reader.next())) {
        odc::api::Span span = frame.span(OdbMetadataDecoder::columnNames(), true);

        if (span == last) {
            buffer.resize(offset + frame.length(), true);
            buffer.copy(frame.encodedData(), frame.length(), offset);
            offset += frame.length();
        }
    }
    if (!frame) {
        eof_ = true;
    }

    eckit::MemoryHandle odbContentHandle(buffer);
    odbContentHandle.openForRead();
    return eckit::message::Message{new OdbContent(odbContentHandle, offset)};
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
