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

#include "eckit/io/Offset.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/io/PeekHandle.h"
#include "eckit/log/Log.h"
#include "eckit/message/Message.h"

#include "metkit/mars/InlineMetaData.h"

//----------------------------------------------------------------------------------------------------------------------

namespace {

// Check if the next object is an InlineMetaData
bool isEnvelope(eckit::PeekHandle& ph) {
    eckit::Buffer buffer(32);
    ph.peek(buffer.data(), buffer.size());
    
    eckit::MemoryStream s(buffer);
    
    if (!s.next()) {
        return false;
    }
    
    std::string klsName;
    s >> klsName;
    return klsName == "InlineMetaData";
}

}  // namespace

//----------------------------------------------------------------------------------------------------------------------

namespace metkit::mars {


// Hopefully wrapping a splitter does not mess things up...
//@todo: It would be nice if I could give codesSplitter only the handle up to the next envelope.
// But then again, my inability to do this is why I am doing it here.
BufrEnvelopeSplitter::BufrEnvelopeSplitter(eckit::PeekHandle& handle) : Splitter(handle), codesSplitter_(handle) {}

BufrEnvelopeSplitter::~BufrEnvelopeSplitter() {}

void BufrEnvelopeSplitter::consumeEnvelope() {

    /// @todo: hardcoded guess for maximum metadata request size. 1MB may be excessive.
    eckit::Buffer buffer(1024 * 1024);  
    handle_.peek(buffer.data(), buffer.size());

    // Deserialise the inline metadata
    eckit::MemoryStream ss(buffer);
    InlineMetaData metadata{ss};
    eckit::Log::info() << "Received InlineMetaData: " << metadata << std::endl;

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
