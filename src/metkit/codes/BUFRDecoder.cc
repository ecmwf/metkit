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

#include "metkit/codes/BUFRDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit {
namespace codes {

namespace {
class HandleDeleter {
    codes_handle *h_;
public:
    HandleDeleter(codes_handle *h) : h_(h) {}
    ~HandleDeleter() {
        if (h_) {
            codes_handle_delete(h_);
        }
    }

    codes_handle* get() { return h_; }

};
}
//----------------------------------------------------------------------------------------------------------------------

bool BUFRDecoder::match(const eckit::message::Message& msg) const {
    size_t len    = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and p[0] == 'B' and p[1] == 'U' and p[2] == 'F' and p[3] == 'R';
}

// mars::MarsRequest BUFRDecoder::messageToRequest(const eckit::message::Message& msg) const {
//     static std::string gribToRequestNamespace =
//         eckit::Resource<std::string>("gribToRequestNamespace", "mars");



//     codes_handle* h = codes_handle_new_from_message(nullptr, msg.data(), msg.length());
//     ASSERT(h);
//     HandleDeleter d(h);

//     mars::MarsRequest r("bufr");

//     codes_keys_iterator* ks = codes_keys_iterator_new(h,
//                               GRIB_KEYS_ITERATOR_ALL_KEYS,
//                               gribToRequestNamespace.c_str());

//     ASSERT(ks);

//     while (codes_keys_iterator_next(ks)) {
//         const char* name = codes_keys_iterator_get_name(ks);

//         if (name[0] == '_') {
//             continue;
//         }

//         char value[1024];
//         size_t len = sizeof(value);

//         ASSERT(codes_keys_iterator_get_string(ks, value, &len) == 0);
//         if (*value) {
//             r.setValue(name, value);
//         }
//     }

//     codes_keys_iterator_delete(ks);

//     return r;

void BUFRDecoder::getMetadata(const eckit::message::Message& msg,
                              eckit::message::MetadataGatherer&) const {
    NOTIMP;
}


void BUFRDecoder::print(std::ostream& s) const {
    s << "BUFRDecoder[]";
}


static BUFRDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
