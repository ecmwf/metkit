/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/CodesDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "eckit/io/Buffer.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/utils/StringTools.h"

#include "metkit/codes/CodesHandleDeleter.h"
#include "metkit/codes/GRIBDecoder.h"

#include <algorithm>
#include <iostream>

#include "eccodes.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

bool GRIBDecoder::match(const eckit::message::Message& msg) const {
    size_t len    = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and ((p[0] == 'G' and p[1] == 'R' and p[2] == 'I' and p[3] == 'B') or
                         (p[0] == 'T' and p[1] == 'I' and p[2] == 'D' and p[3] == 'E') or
                         (p[0] == 'B' and p[1] == 'U' and p[2] == 'D' and p[3] == 'G'));
}


void GRIBDecoder::getMetadata(const eckit::message::Message& msg, eckit::message::MetadataGatherer& gather,
                              const eckit::message::GetMetadataOptions& options) const {
    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");
    std::string nameSpace                     = options.nameSpace ? *options.nameSpace : gribToRequestNamespace;

    std::unique_ptr<codes_handle> h(::codes_handle_new_from_message(nullptr, msg.data(), msg.length()));
    ASSERT(h);

    std::unique_ptr<codes_keys_iterator> itCtx(::codes_keys_iterator_new(h.get(), 0, nameSpace.c_str()));
    ASSERT(itCtx);

    while (::codes_keys_iterator_next(itCtx.get())) {
        const char* name = ::codes_keys_iterator_get_name(itCtx.get());

        if (name[0] == '_')
            continue;  // skip silly underscores in GRIB

        size_t klen = 0;

        /* get key size to see if it is an array */
        ASSERT(::codes_get_size(h.get(), name, &klen) == 0);
        if (klen != 1) {
            continue;
        }

        decodeKey(h.get(), itCtx.get(), name, gather, options);
    }

    // Explicit override for param (kludge for paramID handling)
    {
        char value[1024];
        size_t len = sizeof(value);
        if (::codes_get_string(h.get(), "paramId", value, &len) == 0) {
            gather.setValue("param", value);
        }
    }

    // Look for request embbeded in GRIB message
    long local;
    size_t size;
    if (::codes_get_long(h.get(), "localDefinitionNumber", &local) == 0 && local == 191) {
        /* TODO: Not grib2 compatible, but speed-up process */
        if (::codes_get_size(h.get(), "freeFormData", &size) == 0 && size != 0) {
            eckit::Buffer buffer(size);
            ASSERT(::codes_get_bytes(h.get(), "freeFormData", static_cast<unsigned char*>(buffer.data()), &size) == 0);

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
    std::size_t size = msg.getSize("values");
    eckit::Buffer buf(size * sizeof(double));

    msg.getDoubleArray("values", reinterpret_cast<double*>(buf.data()), size);
    return buf;
}

std::string GRIBDecoder::getString(codes_handle*, codes_keys_iterator* it, const char*) {
    char val[1024];
    size_t len = sizeof(val);
    // n.b. Do not rely on the returned len value. It is inconsistent (sometimes includes '\0', sometimes doesn't,
    //      and sometimes doesn't get updated at all).
    ASSERT(::codes_keys_iterator_get_string(it, val, &len) == 0);
    return eckit::StringTools::trim(std::string{val});
}

long GRIBDecoder::getLong(codes_handle* h, codes_keys_iterator* it, const char*) {
    size_t len = 1;
    long val;
    ASSERT(::codes_keys_iterator_get_long(it, &val, &len) == 0);
    return val;
}

double GRIBDecoder::getDouble(codes_handle*, codes_keys_iterator* it, const char*) {
    double val;
    size_t len = 1;
    ASSERT(::codes_keys_iterator_get_double(it, &val, &len) == 0);
    return val;
}

bool GRIBDecoder::getBytes(codes_handle*, codes_keys_iterator* it, const char*, unsigned char* vals, size_t* len) {
    return ::codes_keys_iterator_get_bytes(it, vals, len) == 0;
}


void GRIBDecoder::print(std::ostream& s) const {
    s << "GRIBDecoder[]";
}

//----------------------------------------------------------------------------------------------------------------------

static GRIBDecoder gribDecoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
