/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "BufrToRequest.h"

#include "eccodes.h"

#include "eckit/config/Resource.h"
#include "eckit/config/ResourceMgr.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/utils/StringTools.h"

#include "metkit/MarsRequest.h"

namespace metkit {
namespace bufr {

//----------------------------------------------------------------------------------------------------------------------


#define MAX_VAL_LEN 1024


#if 0
static void list_all_kvs(codes_handle* h) {
    // CODES_CHECK(codes_set_long(h, "unpack", 1), nullptr); // tell ecCodes to unpack the data
    // values

    codes_bufr_keys_iterator* kiter = codes_bufr_keys_iterator_new(h, CODES_KEYS_ITERATOR_ALL_KEYS);
    ASSERT(kiter);

    while (codes_bufr_keys_iterator_next(kiter)) {
        char* name = codes_bufr_keys_iterator_get_name(kiter);

        int keyType = 0;
        CODES_CHECK(codes_get_native_type(h, name, &keyType), nullptr);

        size_t klen = 0;s
        CODES_CHECK(codes_get_size(h, name, &klen), nullptr);

        if (klen == 1) /* not array */
        {
            size_t vlen             = MAX_VAL_LEN;
            char value[MAX_VAL_LEN] = {0};
            codes_get_string(h, name, value, &vlen);
            eckit::Log::info() << name << " : " << value << std::endl;
        }
        else { /* for arrays */
            eckit::Log::info() << name << " : array(" << klen << ")" << std::endl;
        }
    }

    codes_bufr_keys_iterator_delete(kiter);
}
#endif

static void list_namespace_kvs(codes_handle* h, const char* namespc) {
    char value[128] = {0};

    codes_keys_iterator* ks = codes_keys_iterator_new(h, CODES_KEYS_ITERATOR_ALL_KEYS, namespc);
    ASSERT(ks);

    while (codes_keys_iterator_next(ks)) {
        const char* name = codes_keys_iterator_get_name(ks);

        if (name[0] == '_') {
            continue;
        }

        size_t len = sizeof(value);
        ASSERT(codes_keys_iterator_get_string(ks, value, &len) == 0);
        eckit::Log::info() << name << " : " << value << std::endl;
    }
}


void BufrToRequest::handleToRequest(const bufr::BufrHandle& handle, MarsRequest& req) {
    static std::string bufrToRequestNamespace =
        eckit::Resource<std::string>("bufrToRequestNamespace", "mars");

    BufrHandle::keys_t keys = handle.keys(bufrToRequestNamespace.c_str());

    eckit::Log::info() << "edition " << handle.edition() << " keys: " << keys << std::endl;
}

void BufrToRequest::messageToRequest(const void* buffer, size_t length, MarsRequest& req) {
    BufrHandle handle(buffer, length);

    BufrToRequest::handleToRequest(handle, req);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace bufr
}  // namespace metkit
