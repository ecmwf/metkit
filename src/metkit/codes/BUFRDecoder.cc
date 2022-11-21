/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eccodes.h"

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

namespace {

// TODO In C++14: move to lambda with auto
struct BUFRMetadataIt {
    codes_handle* h;
    codes_bufr_keys_iterator* itCtx;
    eckit::message::MetadataGatherer& gather;

    template <typename DecFunc>
    void operator()(DecFunc&& decode) {
        while (codes_bufr_keys_iterator_next(itCtx)) {
            const char* name = codes_bufr_keys_iterator_get_name(itCtx);

            if (strcmp(name, "subsetNumber") == 0)
                continue;

            size_t klen = 0;

            /* get key size to see if it is an array */
            ASSERT(codes_get_size(h, name, &klen) == 0);
            if (klen != 1) {
                continue;
            }
            decode(h, gather, name);
        }
    }
};

}  // namespace


void BUFRDecoder::getMetadata(const eckit::message::Message& msg,
                              eckit::message::MetadataGatherer& gather,
                              const eckit::message::GetMetadataOptions& options) const {
    codes_handle* h = codes_handle_new_from_message(nullptr, msg.data(), msg.length());
    ASSERT(h);
    HandleDeleter<codes_handle> handleDeleter(h);


    // BUFR Performance improvement: https://confluence.ecmwf.int/display/UDOC/Performance+improvement+by+skipping+some+keys+-+ecCodes+BUFR+FAQ
    if ((unsigned long)(options.filter & eckit::message::MetadataFilter::IncludeExtraKeyAttributes) != 0) {
        CODES_CHECK(codes_set_long(h, "skipExtraKeyAttributes", 1), 0);
    }
    // we need to instruct ecCodes to unpack the data values: https://confluence.ecmwf.int/display/ECC/bufr_keys_iterator
    CODES_CHECK(codes_set_long(h, "unpack", 1), 0);


    codes_bufr_keys_iterator* itCtx = codes_bufr_keys_iterator_new(h, metadataFilterToEccodes(options.filter));
    ASSERT(itCtx);
    HandleDeleter<codes_bufr_keys_iterator> itDeleter(itCtx);


    withSpecializedDecoder(
        options,
        // GetString
        [](codes_handle* h, const char* name, char* val, size_t* len) {
            return codes_get_string(h, name, val, len);
        },
        // GetLong
        [](codes_handle* h, const char* name, long* l, size_t* len) {
            return codes_get_long(h, name, l);
        },
        // GetDouble
        [](codes_handle* h, const char* name, double* d, size_t* len) {
            return codes_get_double(h, name, d);
        },
        // GetBytes
        [](codes_handle* h, const char* name, unsigned char* c, size_t* len) {
            return codes_get_bytes(h, name, c, len);
        },
        // Iteration logic
        BUFRMetadataIt{h, itCtx, gather});
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
