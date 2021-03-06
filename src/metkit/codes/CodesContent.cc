/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jun 2020


#include "metkit/codes/CodesContent.h"
// #include "metkit/codes/MessageContent.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"
// #include "metkit/mars/MarsRequest.h"
// #include "metkit/codes/Decoder.h"

#include "metkit/grib/GribHandle.h"


#include "eccodes.h"

namespace metkit {
namespace codes {

CodesContent::CodesContent(codes_handle* handle, bool delete_handle):
    handle_(handle),
    delete_handle_(delete_handle) {
    ASSERT(handle_);
}

CodesContent::CodesContent(const codes_handle* handle):
    CodesContent(const_cast<codes_handle*>(handle), false) {

}


CodesContent::~CodesContent() {
    if (delete_handle_) {
        codes_handle_delete(handle_);
    }
}

size_t CodesContent::length() const {
    size_t size;
    const void* data;
    CODES_CALL(codes_get_message(handle_, &data, &size));
    return size;
}

void CodesContent::write(eckit::DataHandle& handle) const {
    size_t size;
    const void* data;
    CODES_CALL(codes_get_message(handle_, &data, &size));
    if (handle.write(data, size) != size) {
        std::ostringstream oss;
        oss << "Write error to data handle " << handle;
        throw eckit::WriteError(oss.str(), Here());
    }
}

eckit::DataHandle* CodesContent::readHandle() const {
    size_t size;
    const void* data;
    CODES_CALL(codes_get_message(handle_, &data, &size));
    return new eckit::MemoryHandle(data, size);
}

void CodesContent::print(std::ostream & s) const {
    s << "CodesContent[]";
}

std::string CodesContent::getString(const std::string& key) const {
    char values[10240];
    size_t len = sizeof(values);

    values[0] = 0;

    CODES_CALL(codes_get_string(handle_, key.c_str(), values, &len));
    // ASSERT(err)

    return values;
}

long CodesContent::getLong(const std::string& key) const {
    long v = 0;
    CODES_CALL(codes_get_long(handle_, key.c_str(), &v));
    return v;
}

double CodesContent::getDouble(const std::string& key) const {
    double v = 0;
    CODES_CALL(codes_get_double(handle_, key.c_str(), &v));
    return v;
}

void CodesContent::getDoubleArray(const std::string& key, std::vector<double>& values) const {
    size_t size = 0;
    CODES_CALL(codes_get_size(handle_, key.c_str(), &size));

    size_t count = size;
    values.resize(count);
    CODES_CALL(codes_get_double_array(handle_, key.c_str(), &values[0], &count));
    ASSERT(count == size);
}

void CodesContent::setString(const std::string& key, const std::string& value) {
    size_t size = value.size();
    CODES_CALL(codes_set_string(handle_, key.c_str(), value.c_str(), &size));
}

eckit::Offset CodesContent::offset() const {
    long pos;
    CODES_CALL(codes_get_long(handle_, "offset", &pos));
    return pos;
}

const codes_handle* CodesContent::codesHandle() const {
    return handle_;
}

const void* CodesContent::data() const {
    size_t size;
    const void* data;
    CODES_CALL(codes_get_message(handle_, &data, &size));
    return data;
}



}  // namespace close
}  // namespace metkit

