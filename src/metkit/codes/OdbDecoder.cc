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
#include "metkit/codes/Message.h"
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
                                          "odbMarsRequestMapping", "~metkit/share/metkit/odb/marsrequest.yaml"));
    eckit::YAMLConfiguration config(configPath);
    for (const auto& key : config.keys()) {
        eckit::Log::info() << "mapping " << config.getString(key) << " - " << key << std::endl;
        mapping[config.getString(key)] = key;
    }
}


//----------------------------------------------------------------------------------------------------------------------
bool OdbDecoder::match(const Message& msg) const {
    size_t len = msg.length();
    const unsigned char* p = static_cast<const unsigned char*>(msg.data());
    return len >= 5 and (
               (p[0] == 0xff and p[1] == 0xff and p[2] == 'O' and p[3] == 'D' and p[4] == 'A')
           );
}


class MarsRequestSetter : public odc::api::SpanVisitor {

    virtual void operator()(const std::string& columnName, const std::set<long>& vals) {
        std::cout << "SETTER (long) " << columnName << " - " << vals << std::endl;
    }

    virtual void operator()(const std::string& columnName, const std::set<double>& vals) {
        std::cout << "SETTER (double) " << columnName << " - " << vals << std::endl;
    }

    virtual void operator()(const std::string& columnName, const std::set<std::string>& vals) {
        std::cout << "SETTER (string) " << columnName << " - " << vals << std::endl;
    }

};


mars::MarsRequest OdbDecoder::messageToRequest(const Message& message) const {

    pthread_once(&once, init);

    std::cout << "messageToRequest ===> " << message.length() << std::endl;


    mars::MarsRequest result("odb");
    return result;

    std::unique_ptr<eckit::DataHandle> handle(message.readHandle());
    handle->openForRead();
    eckit::AutoClose close(*handle);


    odc::api::Reader reader(*handle);
    odc::api::Frame frame(reader);

    bool onlyConstantColumns = false;

    // std::vector<MarsRequest> requests;

    std::vector<std::string> columnNames;
    columnNames.reserve(mapping.size());
    for (const auto& kv : mapping) {
        columnNames.push_back(kv.first);
    }

    // MarsLanguage language(verb_);

    MarsRequestSetter setter;

    while (frame.next(false)) {
        odc::api::Span span = frame.span(columnNames, onlyConstantColumns);

        // MarsRequest r(verb_);
        // MarsRequestSetter setter(r, language, mapping_);
        span.visit(setter);

        // if (one_ and requests.size()) {
        //     requests.back().merge(r);
        // }
        // else {
        //     requests.push_back(r);
        // }
        // break;
    }

    std::cout << "messageToRequest <=== " << std::endl;

    return result;
}

void OdbDecoder::print(std::ostream& s) const {
    s << "OdbDecoder[]";
}


static OdbDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
