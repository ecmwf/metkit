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

#include "metkit/codes/GRIBDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/serialisation/MemoryStream.h"
#include "metkit/codes/Message.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------
bool GRIBDecoder::match(const Message& msg) const {
    size_t len = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and (
               (p[0] == 'G' and p[1] == 'R' and p[2] == 'I' and p[3] == 'B') or
               (p[0] == 'T' and p[1] == 'I' and p[2] == 'D' and p[3] == 'E') or
               (p[0] == 'B' and p[1] == 'U' and p[2] == 'D' and p[3] == 'G')
           );
}

mars::MarsRequest GRIBDecoder::messageToRequest(const Message& msg) const {
    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    const codes_handle* h = msg.codesHandle();

    const char* p = static_cast<const char*>(msg.data());

    mars::MarsRequest r(p[0] == 'G' ? "grib" : (p[0] == 'T' ? "tide" : "budg"));

    codes_keys_iterator *ks = codes_keys_iterator_new(
                                 const_cast<codes_handle*>(h),
                                 GRIB_KEYS_ITERATOR_ALL_KEYS,
                                 gribToRequestNamespace.c_str());

    ASSERT(ks);

    while (codes_keys_iterator_next(ks)) {
        const char *name = codes_keys_iterator_get_name(ks);

        if (name[0] == '_') {
            continue;
        }

        char value[1024];
        size_t len = sizeof(value);

        ASSERT( codes_keys_iterator_get_string(ks, value, &len) == 0);

        if (*value) {
            r.setValue(name, value);
        }

    }

    codes_keys_iterator_delete(ks);

    {
        char value[1024];
        size_t len = sizeof(value);
        if (codes_get_string(h, "paramId", value, &len) == 0) {
            r.setValue("param", value);
        }
    }

    // Look for request embbeded in GRIB message
    long local;
    size_t size;
    if (codes_get_long(h, "localDefinitionNumber", &local) ==  0 && local == 191) {
        /* TODO: Not grib2 compatible, but speed-up process */
        if (codes_get_size(h, "freeFormData", &size) ==  0 && size != 0) {
            unsigned char buffer[size];
            ASSERT(codes_get_bytes(h, "freeFormData", buffer, &size) == 0);

            eckit::MemoryStream s(buffer, size);

            int count;
            s >> count; // Number of requests
            ASSERT(count == 1);
            std::string tmp;
            s >> tmp; // verb
            s >> count;
            for (int i = 0; i < count; i++) {
                std::string keyword, value;
                int n;
                s >> keyword;
                std::transform(keyword.begin(), keyword.end(), keyword.begin(), tolower);
                s >> n; // Number of values
                ASSERT(n == 1);
                s >> value;
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                r.setValue(keyword, value);
            }
        }
    }

    return r;
}

void GRIBDecoder::print(std::ostream& s) const {
    s << "GRIBDecoder[]";
}


static GRIBDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
