/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/GribHandle.h"

#include "eccodes.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/StdFile.h"

#include "metkit/codes/GribAccessor.h"

using namespace std;


namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

void codes_call(int code, const char* msg, const eckit::CodeLocation& where) {
    if (code) {
        std::ostringstream os;
        os << msg << " : " << codes_get_error_message(code);
        throw eckit::Exception(os.str(), where);
    }
}

//----------------------------------------------------------------------------------------------------------------------

GribHandle::GribHandle(const eckit::PathName& path):
    handle_(nullptr),
    owned_(true) {

    eckit::AutoStdFile f(path);

    int err        = 0;
    codes_handle* h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err);
    if (err != 0) {
        std::ostringstream os;
        os << "GribHandle() failed to build from path " << path;
        throw eckit::Exception(os.str(), Here());
    }

    ASSERT(h);
    handle_ = h;
}

GribHandle::GribHandle(codes_handle* h):
    handle_(h),
    owned_(true) {
    ASSERT(h);
}

GribHandle::GribHandle(codes_handle& h):
    handle_(&h),
    owned_(false) {
}

GribHandle::GribHandle(eckit::DataHandle& handle):
    handle_(nullptr),
    owned_(true) {

    codes_handle* h = nullptr;
    int err = 0;

    FILE* f = handle.openf();
    ASSERT(f);

    h = codes_handle_new_from_file(0, f, PRODUCT_GRIB, &err);

    CODES_CALL(err);
    ASSERT(h);
    handle_ = h;

    fclose(f);
}

GribHandle::GribHandle(eckit::DataHandle& handle, eckit::Offset offset):
    handle_(nullptr),
    owned_(true) {

    codes_handle* h = nullptr;
    int err = 0;

    FILE* f = handle.openf();
    ASSERT(f);

    handle.seek(offset);

    h = codes_handle_new_from_file(0, f, PRODUCT_GRIB, &err);

    CODES_CALL(err);
    ASSERT(h);
    handle_ = h;

    fclose(f);
}

GribHandle::~GribHandle() noexcept(false) {
    if (handle_ && owned_) {
        CODES_CALL(codes_handle_delete(handle_));
        handle_ = nullptr;
    }
}

std::string GribHandle::geographyHash() const {
    // The following key is edition independent
    return GribAccessor<std::string>("md5GridSection")(*this);
}

size_t GribHandle::getDataValuesSize() const {
    size_t count = 0;
    CODES_CALL(codes_get_size(raw(), "values", &count));
    return count;
}

void GribHandle::getDataValues(double* values, const size_t& count) const {
    ASSERT(values);
    size_t n = count;
    CODES_CALL(codes_get_double_array(raw(), "values", values, &n));
    ASSERT(n == count);
}

double* GribHandle::getDataValues(size_t& count) const {
    count = getDataValuesSize();

    double* values = new double[count];
    getDataValues(values, count);
    return values;
}

void GribHandle::setDataValues(const double* values, size_t count) {
    ASSERT(values);
    CODES_CALL(codes_set_double_array(raw(), "values", values, count));
}

void GribHandle::dump( const eckit::PathName& path, const char* mode) const {
    eckit::StdFile f(path.localPath(), "w");
    codes_dump_content(handle_, f, "mode", 0, 0);
    f.close();
}

void GribHandle::write(const eckit::PathName& path, const char* mode) const {
    ASSERT(codes_write_message(handle_, path.localPath(), mode) == 0);
}

size_t GribHandle::write(eckit::DataHandle& handle) const {
    const void* message = nullptr;
    size_t length       = 0;

    CODES_CALL(codes_get_message(raw(), &message, &length));

    ASSERT(message);
    ASSERT(length);

    ASSERT(length = long(length));
    ASSERT(handle.write(message, length) == long(length));

    return length;
}

size_t GribHandle::length() const {
    const void* message = nullptr;
    size_t length       = 0;

    CODES_CALL(codes_get_message(raw(), &message, &length));
    return length;
}

size_t GribHandle::write(eckit::Buffer& buff) const {
    size_t len = buff.size();
    CODES_CALL(codes_get_message_copy(raw(), buff, &len));  // will issue error if buffer too small
    return len;
}

GribHandle* GribHandle::clone() const {
    codes_handle* h = codes_handle_clone(raw());
    if (!h) {
        throw eckit::WriteError(std::string("failed to clone output grib"));
    }

    return new GribHandle(h);
}

bool GribHandle::hasKey(const char* key) const {
    return (codes_is_defined(handle_, key) != 0);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace grib
}  // namespace metkit
