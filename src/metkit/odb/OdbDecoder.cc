/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/odb/OdbDecoder.h"

#include <algorithm>

#include "eckit/config/Resource.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/message/Message.h"
#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/io/DataHandle.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/odb/OdbMetadataDecoder.h"


namespace metkit {
namespace codes {


//----------------------------------------------------------------------------------------------------------------------
bool OdbDecoder::match(const eckit::message::Message& msg) const {
    size_t len = msg.length();
    const unsigned char* p = static_cast<const unsigned char*>(msg.data());
    return len >= 5 and (
               (p[0] == 0xff and p[1] == 0xff and p[2] == 'O' and p[3] == 'D' and p[4] == 'A')
           );
}



void OdbDecoder::getMetadata(const eckit::message::Message& msg,
                             eckit::message::MetadataGatherer& gather,
                             const eckit::message::GetMetadataOptions& options) const {

    std::unique_ptr<eckit::DataHandle> handle(msg.readHandle());
    handle->openForRead();
    eckit::AutoClose close(*handle);

    odc::api::Reader reader(*handle, false);
    odc::api::Frame frame;

    OdbMetadataDecoder setter(gather, options);

    while ((frame = reader.next())) {
        odc::api::Span span = frame.span(OdbMetadataDecoder::columnNames(), true);

        span.visit(setter);
    }

}

eckit::Buffer OdbDecoder::decode(const eckit::message::Message& msg) const {
    NOTIMP; // Not relevant for MultIO hackathon. Implement as needed in future.
}

void OdbDecoder::print(std::ostream& s) const {
    s << "OdbDecoder[]";
}


static OdbDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
