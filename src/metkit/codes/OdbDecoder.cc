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
#include <sstream>
#include <iomanip>

#include "eccodes.h"

#include "metkit/codes/OdbDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/message/Message.h"
#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/io/DataHandle.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/odb/IdMapper.h"
#include "metkit/fields/FieldIndexGatherer.h"


namespace metkit {
namespace codes {

pthread_once_t once = PTHREAD_ONCE_INIT;


static std::map<std::string, std::string> mapping;
static std::vector<std::string> columnNames;

static void init() {
    static eckit::PathName configPath(eckit::Resource<eckit::PathName>(
                                          "odbMarsRequestMapping",
                                          "~metkit/share/metkit/odb/marsrequest.yaml"));
    eckit::YAMLConfiguration config(configPath);
    for (const auto& key : config.keys()) {
        // eckit::Log::info() << "mapping " << config.getString(key) << " - " << key << std::endl;
        mapping[config.getString(key)] = eckit::StringTools::lower(key);        
    }

    columnNames.reserve(mapping.size());
    for (const auto& kv : mapping) {
        columnNames.push_back(kv.first);
    }
}


//----------------------------------------------------------------------------------------------------------------------

odc::api::Span OdbMetadataSetter::span(odc::api::Frame& frame) {
    pthread_once(&once, init);

    return frame.span(columnNames, true);
}

OdbMetadataSetter::OdbMetadataSetter(eckit::message::MetadataGatherer& gather): gather_(gather) {}

void OdbMetadataSetter::operator()(const std::string& columnName, const std::set<long>& vals) {
    ASSERT(vals.size() == 1);
    std::string strValue;
    if (metkit::odb::IdMapper::instance().alphanumeric(mapping[columnName], *vals.begin(), strValue)) {
        gather_.setValue(mapping[columnName], strValue);
    } else {
        if (mapping[columnName] == "time") {
            std::stringstream ss;
            ss << std::setw(4) << std::setfill('0') << (*vals.begin()/100);
            gather_.setValue(mapping[columnName], ss.str());
        } else {
            gather_.setValue(mapping[columnName], *vals.begin());
        }
    }
}

void OdbMetadataSetter::operator()(const std::string& columnName, const std::set<double>& vals) {
    ASSERT(vals.size() == 1);
    gather_.setValue(mapping[columnName], *vals.begin());
}

void OdbMetadataSetter::operator()(const std::string& columnName, const std::set<std::string>& vals) {
    ASSERT(vals.size() == 1);
    gather_.setValue(mapping[columnName], eckit::StringTools::trim(*vals.begin()));
}


//----------------------------------------------------------------------------------------------------------------------


bool OdbDecoder::match(const eckit::message::Message& msg) const {
    size_t len = msg.length();
    const unsigned char* p = static_cast<const unsigned char*>(msg.data());
    return len >= 5 and (
               (p[0] == 0xff and p[1] == 0xff and p[2] == 'O' and p[3] == 'D' and p[4] == 'A')
           );
}



void OdbDecoder::getMetadata(const eckit::message::Message& msg,
                             eckit::message::MetadataGatherer& gather) const {
    pthread_once(&once, init);

    std::unique_ptr<eckit::DataHandle> handle(msg.readHandle());
    handle->openForRead();
    eckit::AutoClose close(*handle);

    odc::api::Reader reader(*handle, false);
    odc::api::Frame frame;

    fields::FieldIndexGatherer* last = nullptr;
    OdbMetadataSetter setter(gather);

    while ((frame = reader.next())) {
        odc::api::Span span = OdbMetadataSetter::span(frame);

        fields::FieldIndexGatherer* idx  = new fields::FieldIndexGatherer();
        OdbMetadataSetter idxSetter(*idx);
        span.visit(idxSetter);

        if (last) {
            if (*last == *idx) {
                delete idx;
            } else {
                std::stringstream ss;
                ss << "ERROR: Two ODB frames with different MARS metadata in the same message. ABORTING" << std::endl;
                ss << *last;
                ss << *idx;
                eckit::Log::error() << ss.str() << std::endl;
                throw eckit::SeriousBug(ss.str(), Here());
            }
        } else {
            last = idx;
        }

        span.visit(setter);
    }

}


void OdbDecoder::print(std::ostream& s) const {
    s << "OdbDecoder[]";
}


static OdbDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
