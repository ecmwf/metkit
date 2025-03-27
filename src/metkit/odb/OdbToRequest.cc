/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/odb/OdbToRequest.h"

#include <algorithm>

#include "eckit/io/DataHandle.h"
#include "eckit/message/Message.h"

#include "odc/api/Odb.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/odb/OdbMetadataDecoder.h"


using namespace eckit;
using namespace odc::api;

namespace metkit {
using namespace mars;
using namespace codes;

namespace odb {

//----------------------------------------------------------------------------------------------------------------------

class MarsRequestSetter : public eckit::message::MetadataGatherer {
public:  // methods

    MarsRequestSetter(MarsRequest& request) : request_(request) {}

    void setValue(const std::string& key, const std::string& value) override { request_.setValue(key, value); }
    void setValue(const std::string& key, long value) override { request_.setValue(key, value); }
    void setValue(const std::string& key, double value) override { request_.setValue(key, value); }

private:  // members

    MarsRequest& request_;
};

//----------------------------------------------------------------------------------------------------------------------

OdbToRequest::OdbToRequest(const std::string& verb, bool one, bool constant) :
    verb_(verb), one_(one), onlyConstantColumns_(constant) {
    LOG_DEBUG_LIB(LibMetkit) << "OdbToRequest one: " << one << " constant: " << constant << std::endl;
}


OdbToRequest::~OdbToRequest() {}


std::vector<MarsRequest> OdbToRequest::odbToRequest(DataHandle& dh) const {
    LOG_DEBUG_LIB(LibMetkit) << "OdbToRequest::odbToRequest() dh: " << dh << std::endl;

    Reader reader(dh, false);
    Frame frame;

    std::vector<MarsRequest> requests;

    while ((frame = reader.next())) {
        Span span = frame.span(OdbMetadataDecoder::columnNames(), onlyConstantColumns_);

        MarsRequest r(verb_);
        MarsRequestSetter setter(r);
        OdbMetadataDecoder decoder(setter, {}, verb_);
        span.visit(decoder);

        if (one_ and requests.size()) {
            requests.back().merge(r);
        }
        else {
            requests.push_back(r);
        }
    }
    return requests;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace odb
}  // namespace metkit
