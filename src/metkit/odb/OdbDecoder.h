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

#include "eckit/message/Decoder.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

class OdbDecoder : public eckit::message::MessageDecoder {

private:  // methods

    bool match(const eckit::message::Message&) const override;
    void print(std::ostream&) const override;
    void getMetadata(const eckit::message::Message& msg, eckit::message::MetadataGatherer&,
                     const eckit::message::GetMetadataOptions&) const override;

    eckit::Buffer decode(const eckit::message::Message& msg) const override;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
