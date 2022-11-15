/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/codes/Decoder.h"

#include "metkit/codes/BUFRDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "eckit/parser/YAMLParser.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

static std::map<long, long> subtypes_;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void readTable() {
    eckit::PathName bufrSubtypesPath = eckit::Resource<eckit::PathName>("bufrSubtypesPath;$BUFR_SUBTYPES_PATH",
                                                                        LibMetkit::bufrSubtypesYamlFile());

    const eckit::Value bufrSubtypes = eckit::YAMLParser::decodeFile(bufrSubtypesPath);
    const eckit::Value subtypes     = bufrSubtypes["subtypes"];
    ASSERT(subtypes.isList());
    for (size_t i = 0; i < subtypes.size(); ++i) {
        const eckit::Value s = subtypes[i];
        ASSERT(s.isList());
        ASSERT(s.size() == 2);
        subtypes_[s[0]] = s[1];
    }
}

bool BUFRDecoder::typeBySubtype(long subtype, long& type) {
    pthread_once(&once, readTable);

    auto s = subtypes_.find(subtype);
    if (s != subtypes_.end()) {
        type = s->second;
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool BUFRDecoder::match(const eckit::message::Message& msg) const {
    size_t len    = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and p[0] == 'B' and p[1] == 'U' and p[2] == 'F' and p[3] == 'R';
}


//----------------------------------------------------------------------------------------------------------------------

void BUFRDecoder::getMetadata(const eckit::message::Message& msg,
                              eckit::message::MetadataGatherer& gather,
                              const eckit::message::GetMetadataOptions& options) const {
    using ItCtx                           = codes_bufr_keys_iterator;
    eckit::message::MetadataFilter filter = options.filter;

    ::metkit::codes::getMetadata(
        msg, gather, options,
        // Init it
        [filter](codes_handle* h) {
            // BUFR Performance improvement: https://confluence.ecmwf.int/display/UDOC/Performance+improvement+by+skipping+some+keys+-+ecCodes+BUFR+FAQ
            if ((unsigned long)(filter & eckit::message::MetadataFilter::IncludeExtraKeyAttributes) != 0) {
                CODES_CHECK(codes_set_long(h, "skipExtraKeyAttributes", 1), 0);
            }
            // we need to instruct ecCodes to unpack the data values: https://confluence.ecmwf.int/display/ECC/bufr_keys_iterator
            CODES_CHECK(codes_set_long(h, "unpack", 1), 0);

            return codes_bufr_keys_iterator_new(h, metadataFilterToEccodes(filter));
        },
        // Iterator next
        [](codes_handle* h, ItCtx* ks) {
            while (codes_bufr_keys_iterator_next(ks)) {
                const char* name = codes_bufr_keys_iterator_get_name(ks);

                if (strcmp(name, "subsetNumber") == 0)
                    continue;  // skip silly underscores in GRIB

                return eckit::Optional<std::string>(name);
            }
            return eckit::Optional<std::string>{};
        },
        // GetString
        [](codes_handle* h, ItCtx*, const std::string& name, char* val, size_t* len) {
            return codes_get_string(h, name.c_str(), val, len);
        },
        // GetLong
        [](codes_handle* h, ItCtx*, const std::string& name, long* l, size_t* len) {
            return codes_get_long(h, name.c_str(), l);
        },
        // GetDouble
        [](codes_handle* h, ItCtx*, const std::string& name, double* d, size_t* len) {
            return codes_get_double(h, name.c_str(), d);
        },
        // GetBytes
        [](codes_handle* h, ItCtx*, const std::string& name, unsigned char* c, size_t* len) {
            return codes_get_bytes(h, name.c_str(), c, len);
        },
        // Post Process
        [](codes_handle*, eckit::message::MetadataGatherer&, ItCtx*) {
        });
}


//----------------------------------------------------------------------------------------------------------------------

eckit::Buffer BUFRDecoder::decode(const eckit::message::Message& msg) const {
    std::size_t size = msg.getSize("numericValues");
    eckit::Buffer buf(size * sizeof(double));

    msg.getDoubleArray("numericValues", reinterpret_cast<double*>(buf.data()), size);
    return buf;
}


//----------------------------------------------------------------------------------------------------------------------

eckit::message::EncodingFormat BUFRDecoder::getEncodingFormat(const eckit::message::Message& msg) const {
    return eckit::message::EncodingFormat::BUFR;
};


//----------------------------------------------------------------------------------------------------------------------

void BUFRDecoder::print(std::ostream& s) const {
    s << "BUFRDecoder[]";
}


//----------------------------------------------------------------------------------------------------------------------

static BUFRDecoder bufrDecoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
