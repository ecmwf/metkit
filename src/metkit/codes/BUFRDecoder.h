// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @date   Jun 2020

#pragma once

#include "eckit/message/Decoder.h"
#include "eckit/message/Message.h"

#include "eckit/io/Buffer.h"


namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

class BUFRDecoder : public eckit::message::MessageDecoder {

public:  // methods

    static bool typeBySubtype(long subtype, long& type);

private:  // methods

    bool match(const eckit::message::Message&) const override;
    void print(std::ostream&) const override;

    void getMetadata(const eckit::message::Message& msg, eckit::message::MetadataGatherer&,
                     const eckit::message::GetMetadataOptions&) const override;

    eckit::Buffer decode(const eckit::message::Message& msg) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
