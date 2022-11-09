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

#include "eccodes.h"

#include "metkit/codes/GRIBDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "eckit/serialisation/MemoryStream.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit {
namespace codes {

namespace {
class HandleDeleter {
    codes_handle* h_;

public:
    HandleDeleter(codes_handle* h) :
        h_(h) {}
    ~HandleDeleter() {
        if (h_) {
            codes_handle_delete(h_);
        }
    }

    codes_handle* get() { return h_; }
};
}  // namespace

//----------------------------------------------------------------------------------------------------------------------
bool GRIBDecoder::match(const eckit::message::Message& msg) const {
    size_t len    = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and ((p[0] == 'G' and p[1] == 'R' and p[2] == 'I' and p[3] == 'B') or (p[0] == 'T' and p[1] == 'I' and p[2] == 'D' and p[3] == 'E') or (p[0] == 'B' and p[1] == 'U' and p[2] == 'D' and p[3] == 'G'));
}


void GRIBDecoder::getMetadata(const eckit::message::Message& msg,
                              eckit::message::MetadataGatherer& gather, eckit::message::ValueRepresentation repr) const {

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");


    codes_handle* h = codes_handle_new_from_message(nullptr, msg.data(), msg.length());
    ASSERT(h);
    HandleDeleter d(h);


    codes_keys_iterator* ks = codes_keys_iterator_new(h,
                                                      GRIB_KEYS_ITERATOR_ALL_KEYS,
                                                      gribToRequestNamespace.c_str());

    ASSERT(ks);

    while (codes_keys_iterator_next(ks)) {
        const char* name = codes_keys_iterator_get_name(ks);

        if (name[0] == '_')
            continue;  // skip silly underscores in GRIB

        size_t klen = 0;
        /* get key size to see if it is an array */
        ASSERT(codes_get_size(h, name, &klen) == 0);

        if (klen != 1) {
            continue;
        }

        const auto decodeString = [&gather, ks, name]() {
            char val[1024];
            size_t len = sizeof(val);
            ASSERT(codes_keys_iterator_get_string(ks, val, &len) == 0);
            if (*val) {
                gather.setValue(name, val);
                return true;
            }
            return false;
        };
        const auto decodeLong = [&gather, ks, name]() {
            long l;
            size_t len = 1;
            if (codes_keys_iterator_get_long(ks, &l, &len) == 0) {
                gather.setValue(name, l);
                return true;
            }
            return false;
        };
        const auto decodeDouble = [&gather, ks, name]() {
            double d;
            size_t len = 1;
            if (codes_keys_iterator_get_double(ks, &d, &len) == 0) {
                gather.setValue(name, d);
                return true;
            }
            return false;
        };

        const auto decodeNative = [h, name, &decodeLong, &decodeString, &decodeDouble]() {
            int keyType = 0;
            ASSERT(codes_get_native_type(h, name, &keyType) == 0);
            // GRIB_ Type prefixes are also valid for BUFR
            switch (keyType) {
                case GRIB_TYPE_LONG: {
                    return decodeLong();
                }
                case GRIB_TYPE_DOUBLE: {
                    return decodeDouble();
                }
                case GRIB_TYPE_STRING: {
                    return decodeString();
                }
                default: {
                    return decodeDouble() || decodeLong() || decodeString();
                }
            }
            return true;
        };

        switch (repr) {
            case eckit::message::ValueRepresentation::Native: {
                decodeNative();
                break;
            }
            case eckit::message::ValueRepresentation::String: {
                decodeString() || decodeNative();
                break;
            }
            case eckit::message::ValueRepresentation::Numeric: {
                decodeDouble() || decodeLong() || decodeNative();
                break;
            }
        }
    }

    codes_keys_iterator_delete(ks);

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


eckit::Buffer GRIBDecoder::decode(const eckit::message::Message& msg) const {
    std::vector<double> v;
    // @TODO Is this valid for all GRIB messages? Worked with results from mars
    msg.getDoubleArray("values", v);

    return eckit::Buffer(reinterpret_cast<void*>(v.data()), v.size() * sizeof(double));
}

eckit::message::EncodingFormat GRIBDecoder::getEncodingFormat(const eckit::message::Message& msg) const {
    return eckit::message::EncodingFormat::GRIB;
};


void GRIBDecoder::print(std::ostream& s) const {
    s << "GRIBDecoder[]";
}


static GRIBDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
