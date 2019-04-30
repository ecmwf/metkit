/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "BufrHandle.h"

#include "eccodes.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/StdFile.h"

using namespace std;

namespace metkit {
namespace bufr {

//----------------------------------------------------------------------------------------------------------------------

void bufr_call(int code, const char* msg, const eckit::CodeLocation& where) {
    if (code) {
        std::ostringstream os;
        os << msg << " : " << codes_get_error_message(code);
        throw eckit::Exception(os.str(), where);
    }
}

//----------------------------------------------------------------------------------------------------------------------

BufrHandle::BufrHandle(const eckit::PathName& path) {
    eckit::AutoStdFile f(path);

    int err = 0;

    codes_handle* h = codes_handle_new_from_file(ctxt_, f, PRODUCT_BUFR, &err);
    if (err != 0) {
        std::ostringstream os;
        os << "BufrHandle() failed to build from path " << path;
        throw eckit::Exception(os.str(), Here());
    }

    ASSERT(h);
    handle_ = h;
    owned_  = true;
}

BufrHandle::BufrHandle(codes_handle* h) {
    ASSERT(h);
    handle_ = h;
    owned_  = true;
}

BufrHandle::BufrHandle(codes_handle& h) : handle_(&h), owned_(false) {}

BufrHandle::BufrHandle(const eckit::Buffer& buffer, bool copy) {
    init(buffer, buffer.size(), copy);
}

BufrHandle::BufrHandle(const void* buffer, size_t length, bool copy) {
    init(static_cast<const char*>(buffer), length, copy);
}

void BufrHandle::init(const char* buff, size_t len, bool copy) {
    const char* message = buff;
    ASSERT(strncmp(message, "BUFR", 4) == 0);
    ASSERT(handle_ == nullptr);

    if (copy) {
        handle_ = codes_handle_new_from_message_copy(ctxt_, const_cast<char*>(message), len);
    }
    else {
        handle_ = codes_handle_new_from_message(ctxt_, const_cast<char*>(message), len);
    }

    ASSERT(handle_);
    owned_ = true;
}

BufrHandle::~BufrHandle() noexcept(false) {
    if (handle_ && owned_) {
        BUFR_CALL(codes_handle_delete(handle_));
        handle_ = nullptr;
        owned_  = false;
    }
}

long BufrHandle::edition() const {
    long edition;
    BUFR_CALL(codes_get_long(handle_, "editionNumber", &edition));
    return edition;
}

BufrHandle::keys_t BufrHandle::keys(const char* namespc) const {

    ASSERT(handle_);

    BufrHandle::keys_t result;

    char value[128] = {0};

    codes_keys_iterator* ks =
        codes_keys_iterator_new(handle_, CODES_KEYS_ITERATOR_ALL_KEYS, namespc);
    ASSERT(ks);

    while (codes_keys_iterator_next(ks)) {
        const char* name = codes_keys_iterator_get_name(ks);

//                if (name[0] == '_') {
//                    continue;
//                }

        size_t len = sizeof(value);
        ASSERT(codes_keys_iterator_get_string(ks, value, &len) == 0);

        result[name] = value;
    }

    codes_keys_iterator_delete(ks);

    return result;
}

void BufrHandle::getLong(const std::string& k, long& v) const {
    BUFR_CALL(codes_get_long(handle_, k.c_str(), &v));
}

void BufrHandle::getString(const std::string& k, std::string& v) const {
    char value[256] = {0};
    size_t len = sizeof (value);
    BUFR_CALL(codes_get_string(handle_, k.c_str(), value, &len));
    v = value;
}

bool BufrHandle::hasKey(const char* key) const {
    return (codes_is_defined(handle_, key) != 0);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace bufr
}  // namespace metkit
