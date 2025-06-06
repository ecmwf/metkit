/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/BufrEnvelopeSplitter.h"
#include "metkit/config/LibMetkit.h"

#include "eckit/io/Offset.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/io/PeekHandle.h"
#include "eckit/log/Log.h"
#include "eckit/message/Message.h"
#include "eckit/utils/Literals.h"

#include "metkit/mars/InlineMetaData.h"

//----------------------------------------------------------------------------------------------------------------------

using namespace eckit::literals;

namespace {

/// @TODO: I think it would be better for eckit::stream to expose tag-checking functionality
// but as is, the closest function s.next() always writes to stderr if no tag is found,
// which is not acceptable.

inline bool isTagStartObject(char c) {
    return c == 1;
}

inline bool isTagStartString(char c) {
    return c == 15;
}


// Check if the next object is an InlineMetaData
bool isEnvelope(eckit::PeekHandle& ph) {

    eckit::Buffer buffer(32);
    size_t len = ph.peek(buffer.data(), buffer.size());

    if (len < 16 || !isTagStartObject(buffer[0]) || !isTagStartString(buffer[1])) {
        return false;
    }

    eckit::MemoryStream s(buffer);

    s.next();
    
    std::string klsName;
    s >> klsName;

    return klsName == "InlineMetaData";
}

}  // namespace

//----------------------------------------------------------------------------------------------------------------------

namespace metkit::mars {

BufrEnvelopeSplitter::BufrEnvelopeSplitter(eckit::PeekHandle& handle) : Splitter(handle), codesSplitter_(handle) {}

BufrEnvelopeSplitter::~BufrEnvelopeSplitter() {}

void BufrEnvelopeSplitter::consumeEnvelope() {

    /// @todo: hardcoded guess for maximum metadata request size. Sensible or too large?
    eckit::Buffer buffer(1_MiB);  
    handle_.peek(buffer.data(), buffer.size());

    eckit::MemoryStream ss(buffer);
    InlineMetaData metadata{ss};
    LOG_DEBUG_LIB(LibMetkit) << "Received InlineMetaData: " << metadata << std::endl;

    // Consume the envelope to advance the datahandle.
    handle_.read(buffer.data(), ss.position());
    envelopeEnd_ = handle_.position() + metadata.length();
}


/// Return the next message from the codes splitter, but first check if we need to consume an envelope
eckit::message::Message BufrEnvelopeSplitter::next() {

    eckit::Offset pos = handle_.position();
    if (pos >= envelopeEnd_ && isEnvelope(handle_)) {
        consumeEnvelope();
    }
        
    return codesSplitter_.next();
}

void BufrEnvelopeSplitter::print(std::ostream& s) const {
    s << "BufrEnvelopeSplitter[]";
}

}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------

template <>
bool eckit::message::SplitterBuilder<metkit::mars::BufrEnvelopeSplitter>::match(eckit::PeekHandle& handle) const {
   return isEnvelope(handle);
}

static eckit::message::SplitterBuilder<metkit::mars::BufrEnvelopeSplitter> splitter;
