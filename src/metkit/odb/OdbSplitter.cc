// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/odb/OdbSplitter.h"

#include "eckit/io/BufferList.h"
#include "eckit/io/PeekHandle.h"
#include "eckit/message/Message.h"

#include "metkit/config/LibMetkit.h"

#include "metkit/odb/OdbContent.h"
#include "metkit/odb/OdbMetadataDecoder.h"


namespace metkit::codes {

OdbSplitter::OdbSplitter(eckit::PeekHandle& handle) :
    Splitter(handle), handleWrapper_(handle), reader_(handleWrapper_, false) {}

OdbSplitter::~OdbSplitter() {}

eckit::message::Message OdbSplitter::next() {

    if (!lastFrame_) {
        lastFrame_ = reader_.next();
        if (!lastFrame_) {
            return eckit::message::Message();
        }
    }

    eckit::BufferList buffers;

    odc::api::Span reference = lastFrame_.span(OdbMetadataDecoder::columnNames(), true);

    buffers.append(lastFrame_.encodedData());
    handleWrapper_.clear();
    LOG_DEBUG_LIB(LibMetkit) << "ODB frame: " << buffers.count() << ", size: " << lastFrame_.length()
                             << ", total:" << buffers.size() << std::endl;

    lastFrame_ = odc::api::Frame();  //< we have consumed lastFrame_

    odc::api::Frame frame;
    // aggregate all frames with the same metadata Span as reference Span
    while ((frame = reader_.next())) {
        odc::api::Span span = frame.span(OdbMetadataDecoder::columnNames(), true);
        if (span == reference) {
            buffers.append(frame.encodedData());
            handleWrapper_.clear();
            LOG_DEBUG_LIB(LibMetkit) << "ODB frame: " << buffers.count() << ", size: " << frame.length()
                                     << ", total:" << buffers.size() << std::endl;
        }
        else {
            // remember last frame to reuse as reference on the following next()
            lastFrame_ = frame;
            break;
        }
    }

    LOG_DEBUG_LIB(LibMetkit) << "Consolidating buffers of " << buffers.count() << " frames"
                             << ", total size: " << buffers.size() << std::endl;

    return eckit::message::Message{new OdbContent(buffers.consolidate())};
}

void OdbSplitter::print(std::ostream& s) const {
    s << "OdbSplitter[]";
}

}  // namespace metkit::codes

//----------------------------------------------------------------------------------------------------------------------

namespace eckit::message {

template <>
bool SplitterBuilder<metkit::codes::OdbSplitter>::match(eckit::PeekHandle& handle) const {
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

}  // namespace eckit::message

static eckit::message::SplitterBuilder<metkit::codes::OdbSplitter> splitter;
