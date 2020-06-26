/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */



#include <eccodes.h>

#include "metkit/codes/BUFRDecoder.h"

#include "eckit/config/Resource.h"
#include "metkit/codes/Message.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------
bool BUFRDecoder::match(const Message& msg) const {
    size_t len = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and p[0] == 'B' and p[1] == 'U' and p[2] == 'F' and p[3] == 'R';
}

mars::MarsRequest BUFRDecoder::messageToRequest(const Message& msg) const {
    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    const codes_handle* h = msg.codesHandle();

    mars::MarsRequest r("bufr");

    grib_keys_iterator *ks = grib_keys_iterator_new(
                                 const_cast<codes_handle*>(h),
                                 GRIB_KEYS_ITERATOR_ALL_KEYS,
                                 gribToRequestNamespace.c_str());

    ASSERT(ks);

    while (grib_keys_iterator_next(ks)) {
        const char *name = grib_keys_iterator_get_name(ks);

        if (name[0] == '_') {
            continue;
        }

        char value[1024];
        size_t len = sizeof(value);

        ASSERT( grib_keys_iterator_get_string(ks, value, &len) == 0);
        if (*value) {
            r.setValue(name, value);
        }

    }

    grib_keys_iterator_delete(ks);

    return r;

}

void BUFRDecoder::print(std::ostream& s) const {
    s << "BUFRDecoder[]";
}


static BUFRDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
