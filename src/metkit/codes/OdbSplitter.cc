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
#include "eckit/io/Buffer.h"
#include "eckit/io/PeekHandle.h"
#include "eckit/io/MultiHandle.h"
#include "eckit/message/Message.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/codes/OdbContent.h"
#include "metkit/codes/OdbDecoder.h"
#include "metkit/fields/FieldIndexGatherer.h"

#include "odc/api/Odb.h"

namespace metkit {
namespace codes {

OdbSplitter::OdbSplitter(eckit::PeekHandle& handle) : Splitter(handle), nextHandle_(nullptr) {}

OdbSplitter::~OdbSplitter() {}

eckit::message::Message OdbSplitter::next() {
    if (!nextHandle_) {
        nextHandle_ = new eckit::SeekableHandle(handle_);
        nextHandle_->openForRead();
    }


    eckit::Offset offset = nextHandle_->position();
    eckit::Length length = 0;
    fields::FieldIndexGatherer* last = nullptr;

    odc::api::Frame frame;
    odc::api::Reader reader(*nextHandle_, false);

    while ((frame = reader.next())) {
        odc::api::Span span = OdbMetadataSetter::span(frame);

        fields::FieldIndexGatherer* idx  = new fields::FieldIndexGatherer();
        OdbMetadataSetter idxSetter(*idx);
        span.visit(idxSetter);        

        if (last) {
            if (*last == *idx) {
                delete idx;
                length += frame.length();
            } else {
                nextHandle_->seek(offset + length);
                return eckit::message::Message{new OdbContent(handle_, length)};
            }
        } else {
            last = idx;
            length = frame.length();
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
