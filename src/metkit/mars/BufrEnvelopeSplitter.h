/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jun 2020

#pragma once

#include "eckit/io/Offset.h"
#include "eckit/message/Splitter.h"
#include "eckit/io/Length.h"

#include "metkit/codes/CodesSplitter.h"
#include "metkit/mars/InlineMetaData.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class BufrEnvelopeSplitter : public eckit::message::Splitter {
public:

    BufrEnvelopeSplitter(eckit::PeekHandle&);
    ~BufrEnvelopeSplitter();

private:  // methods

    eckit::message::Message next() override;
    void print(std::ostream&) const override;

    void consumeEnvelope();

private: // members

    // This splitter has state: it tracks the length of the current envelope
    // This allows us to add checks that we have consumed the entire envelope
    eckit::Offset envelopeEnd_ = 0;

    metkit::codes::CodesSplitter codesSplitter_; // internal splitter for codes messages

};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
