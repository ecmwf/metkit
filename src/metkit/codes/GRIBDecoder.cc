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

void GRIBDecoder::getMetadata(const eckit::message::Message& msg,
                              eckit::message::MetadataGatherer& gather,
                              const eckit::message::GetMetadataOptions& options) const {
    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");
    std::string nameSpace                     = options.nameSpace ? *options.nameSpace : gribToRequestNamespace;

    using ItCtx            = codes_keys_iterator;
    unsigned long filter   = metadataFilterToEccodes(options.filter);
    const char* cNameSpace = nameSpace.c_str();

    ::metkit::codes::getMetadata(
        msg, gather, options,
        // Init iterator
        [cNameSpace, filter](codes_handle* h) {
            return codes_keys_iterator_new(h, filter, cNameSpace);
        },
        // Iterator next
        [](codes_handle* h, ItCtx* ks) {
            while (codes_keys_iterator_next(ks)) {
                const char* name = codes_keys_iterator_get_name(ks);

                if (name[0] == '_')
                    continue;  // skip silly underscores in GRIB

                return eckit::Optional<std::string>(name);
            }
            return eckit::Optional<std::string>{};
        },
        // GetString
        [](codes_handle* h, ItCtx* ks, const std::string& name, char* val, size_t* len) {
            return codes_keys_iterator_get_string(ks, val, len);
        },
        // GetLong
        [](codes_handle* h, ItCtx* ks, const std::string& name, long* l, size_t* len) {
            return codes_keys_iterator_get_long(ks, l, len);
        },
        // GetDouble
        [](codes_handle* h, ItCtx* ks, const std::string& name, double* d, size_t* len) {
            return codes_keys_iterator_get_double(ks, d, len);
        },
        // GetBytes
        [](codes_handle* h, ItCtx* ks, const std::string& name, unsigned char* c, size_t* len) {
            return codes_keys_iterator_get_bytes(ks, c, len);
        },
        // Post Process
        [](codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* ks) {
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
        });
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
