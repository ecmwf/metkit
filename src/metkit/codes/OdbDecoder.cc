/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>

#include <eccodes.h>

#include "metkit/codes/OdbDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/serialisation/MemoryStream.h"
#include "metkit/data/Message.h"
#include "metkit/mars/MarsRequest.h"
#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/io/DataHandle.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"
#include "odc/api/Odb.h"

namespace metkit {
namespace codes {

pthread_once_t once = PTHREAD_ONCE_INIT;


static std::map<std::string, std::string> mapping;

static void init() {
    static eckit::PathName configPath(eckit::Resource<eckit::PathName>(
                                          "odbMarsRequestMapping",
                                          "~metkit/share/metkit/odb/marsrequest.yaml"));
    eckit::YAMLConfiguration config(configPath);
    for (const auto& key : config.keys()) {
        // eckit::Log::info() << "mapping " << config.getString(key) << " - " << key << std::endl;
        mapping[config.getString(key)] = eckit::StringTools::lower(key);
    }
}


//----------------------------------------------------------------------------------------------------------------------
bool OdbDecoder::match(const data::Message& msg) const {
    size_t len = msg.length();
    const unsigned char* p = static_cast<const unsigned char*>(msg.data());
    return len >= 5 and (
               (p[0] == 0xff and p[1] == 0xff and p[2] == 'O' and p[3] == 'D' and p[4] == 'A')
           );
}


class MarsRequestSetter : public odc::api::SpanVisitor {

    mars::MarsRequest& request_;

public:
    MarsRequestSetter(mars::MarsRequest& request): request_(request) {}

    virtual void operator()(const std::string& columnName, const std::set<long>& vals) {
        ASSERT(vals.size() == 1);
        request_.setValue(mapping[columnName], *vals.begin());
    }

    virtual void operator()(const std::string& columnName, const std::set<double>& vals) {
        ASSERT(vals.size() == 1);
        request_.setValue(mapping[columnName], *vals.begin());
    }

    virtual void operator()(const std::string& columnName, const std::set<std::string>& vals) {
        ASSERT(vals.size() == 1);
        request_.setValue(mapping[columnName], eckit::StringTools::trim(*vals.begin()));
    }

};


mars::MarsRequest OdbDecoder::messageToRequest(const data::Message& message) const {

    pthread_once(&once, init);

    mars::MarsRequest result("odb");

    std::unique_ptr<eckit::DataHandle> handle(message.readHandle());
    handle->openForRead();
    eckit::AutoClose close(*handle);

    odc::api::Reader reader(*handle);
    odc::api::Frame frame(reader);

    std::vector<std::string> columnNames;
    columnNames.reserve(mapping.size());
    for (const auto& kv : mapping) {
        columnNames.push_back(kv.first);
    }

    MarsRequestSetter setter(result);

    while (frame.next(false)) {
        odc::api::Span span = frame.span(columnNames, true);
        span.visit(setter);
    }

    return result;
}

void OdbDecoder::print(std::ostream& s) const {
    s << "OdbDecoder[]";
}


static OdbDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
