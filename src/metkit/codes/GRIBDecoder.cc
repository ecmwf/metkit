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
#include <iomanip>
#include <iostream>

#include "eccodes.h"

#include "metkit/codes/Decoder.h"

#include "metkit/codes/GRIBDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "eckit/serialisation/MemoryStream.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

bool GRIBDecoder::match(const eckit::message::Message& msg) const {
    size_t len    = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4
           and ((p[0] == 'G' and p[1] == 'R' and p[2] == 'I' and p[3] == 'B')
                or (p[0] == 'T' and p[1] == 'I' and p[2] == 'D' and p[3] == 'E')
                or (p[0] == 'B' and p[1] == 'U' and p[2] == 'D' and p[3] == 'G'));
}


//----------------------------------------------------------------------------------------------------------------------

namespace {

// TODO In C++14: move to lambda with auto
struct GRIBMetadataIt {
    codes_handle* h;
    codes_keys_iterator* itCtx;
    eckit::message::MetadataGatherer& gather;

    template <typename DecFunc>
    void operator()(DecFunc&& decode) {
        while (codes_keys_iterator_next(itCtx)) {
            const char* name = codes_keys_iterator_get_name(itCtx);

            if (name[0] == '_')
                continue;  // skip silly underscores in GRIB

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


void GRIBDecoder::getMetadata(const eckit::message::Message& msg,
                              eckit::message::MetadataGatherer& gather,
                              const eckit::message::GetMetadataOptions& options) const {
    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");
    std::string nameSpace                     = options.nameSpace ? *options.nameSpace : gribToRequestNamespace;

    codes_handle* h = codes_handle_new_from_message(nullptr, msg.data(), msg.length());
    ASSERT(h);
    HandleDeleter<codes_handle> handleDeleter(h);

    codes_keys_iterator* itCtx = codes_keys_iterator_new(h, metadataFilterToEccodes(options.filter), nameSpace.c_str());
    ASSERT(itCtx);
    HandleDeleter<codes_keys_iterator> itDeleter(itCtx);

    withSpecializedDecoder(
        options,
        // GetString
        [itCtx](codes_handle* h, const char*, char* val, size_t* len) {
            return codes_keys_iterator_get_string(itCtx, val, len);
        },
        // GetLong
        [itCtx](codes_handle* h, const char*, long* l, size_t* len) {
            return codes_keys_iterator_get_long(itCtx, l, len);
        },
        // GetDouble
        [itCtx](codes_handle* h, const char*, double* d, size_t* len) {
            return codes_keys_iterator_get_double(itCtx, d, len);
        },
        // GetBytes
        [itCtx](codes_handle* h, const char*, unsigned char* c, size_t* len) {
            return codes_keys_iterator_get_bytes(itCtx, c, len);
        },
        GRIBMetadataIt{h, itCtx, gather});


    // Default behaviour for FDB, should be maintained
    {
        char value[1024];
        size_t len = sizeof(value);
        if (codes_get_string(h, "paramId", value, &len) == 0) {
            gather.setValue("param", value);
        }
    }

    // Look for request embbeded in GRIB message
    long local;
    size_t size;
    if (codes_get_long(h, "localDefinitionNumber", &local) == 0 && local == 191) {
        /* TODO: Not grib2 compatible, but speed-up process */
        if (codes_get_size(h, "freeFormData", &size) == 0 && size != 0) {
            unsigned char buffer[size];
            ASSERT(codes_get_bytes(h, "freeFormData", buffer, &size) == 0);

            eckit::MemoryStream s(buffer, size);

            int count;
            s >> count;  // Number of requests
            ASSERT(count == 1);
            std::string tmp;
            s >> tmp;  // verb
            s >> count;
            for (int i = 0; i < count; i++) {
                std::string keyword, value;
                int n;
                s >> keyword;
                std::transform(keyword.begin(), keyword.end(), keyword.begin(), tolower);
                s >> n;  // Number of values
                ASSERT(n == 1);
                s >> value;
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                gather.setValue(keyword, value);
            }
        }
    }
}


//----------------------------------------------------------------------------------------------------------------------

eckit::Buffer GRIBDecoder::decode(const eckit::message::Message& msg) const {
    std::size_t size = msg.getSize("values");
    eckit::Buffer buf(size * sizeof(double));

    msg.getDoubleArray("values", reinterpret_cast<double*>(buf.data()), size);
    return buf;
}


//----------------------------------------------------------------------------------------------------------------------

eckit::message::EncodingFormat GRIBDecoder::getEncodingFormat(const eckit::message::Message& msg) const {
    return eckit::message::EncodingFormat::GRIB;
};


//----------------------------------------------------------------------------------------------------------------------

void GRIBDecoder::print(std::ostream& s) const {
    s << "GRIBDecoder[]";
}


//----------------------------------------------------------------------------------------------------------------------

static GRIBDecoder gribDecoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
