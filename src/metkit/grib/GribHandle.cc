/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/grib/GribHandle.h"

#include "grib_api.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/StdFile.h"

#include "metkit/grib/GribAccessor.h"
#include "metkit/grib/GribDataBlob.h"
#include "metkit/grib/GribMetaData.h"

using namespace std;


namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

void grib_call(int code, const char* msg, const eckit::CodeLocation& where) {
    if (code) {
        std::ostringstream os;
        os << msg << " : " << grib_get_error_message(code);
        throw eckit::Exception(os.str(), where);
    }
}

//----------------------------------------------------------------------------------------------------------------------

GribHandle::GribHandle(const eckit::PathName& path):
    handle_(nullptr),
    owned_(true) {

    eckit::AutoStdFile f(path);

    int err        = 0;
    grib_handle* h = grib_handle_new_from_file(nullptr, f, &err);
    if (err != 0) {
        std::ostringstream os;
        os << "GribHandle() failed to build from path " << path;
        throw eckit::Exception(os.str(), Here());
    }

    ASSERT(h);
    handle_ = h;
}

GribHandle::GribHandle(grib_handle* h):
    handle_(h),
    owned_(true) {
    ASSERT(h);
}

GribHandle::GribHandle(grib_handle& h):
    handle_(&h),
    owned_(false) {
}

GribHandle::GribHandle(eckit::DataHandle& handle, bool partial):
    handle_(nullptr),
    owned_(true) {

    grib_handle* h = nullptr;
    int err = 0;

    FILE* f = handle.openf();
    ASSERT(f);

    if (partial) {
        h = grib_new_from_file(0, f, true, &err);
    }
    else {
        h = grib_handle_new_from_file(0, f, &err);
    }

    GRIB_CALL(err);
    ASSERT(h);
    handle_ = h;
}

GribHandle::~GribHandle() noexcept(false) {
    if (handle_ && owned_) {
        GRIB_CALL(grib_handle_delete(handle_));
        handle_ = nullptr;
    }
}

GribDataBlob* GribHandle::message() const {
    size_t length;
    const void* message;
    GRIB_CALL(grib_get_message(handle_, &message, &length));
    return new GribDataBlob(message, length);
}

std::string GribHandle::geographyHash() const {
    // The following key is edition independent
    return GribAccessor<std::string>("md5GridSection")(*this);
}

size_t GribHandle::getDataValuesSize() const {
    size_t count = 0;
    GRIB_CALL(grib_get_size(raw(), "values", &count));
    return count;
}

void GribHandle::getDataValues(double* values, const size_t& count) const {
    ASSERT(values);
    size_t n = count;
    GRIB_CALL(grib_get_double_array(raw(), "values", values, &n));
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
    GRIB_CALL(grib_set_double_array(raw(), "values", values, count));
}

void GribHandle::dump( const eckit::PathName& path, const char* mode) const {
    eckit::StdFile f(path.localPath(), "w");
    grib_dump_content(handle_, f, "mode", 0, 0);
    f.close();
}

void GribHandle::write(const eckit::PathName& path, const char* mode) const {
    ASSERT(grib_write_message(handle_, path.localPath(), mode) == 0);
}

size_t GribHandle::write(eckit::DataHandle& handle) const {
    const void* message = nullptr;
    size_t length       = 0;

    GRIB_CALL(grib_get_message(raw(), &message, &length));

    ASSERT(message);
    ASSERT(length);

    ASSERT(length = long(length));
    ASSERT(handle.write(message, length) == long(length));

    return length;
}

size_t GribHandle::write(eckit::Buffer& buff) const {
    size_t len = buff.size();
    GRIB_CALL(grib_get_message_copy(raw(), buff, &len));  // will issue error if buffer too small
    return len;
}

GribHandle* GribHandle::clone() const {
    grib_handle* h = grib_handle_clone(raw());
    if (!h) {
        throw eckit::WriteError(std::string("failed to clone output grib"));
    }

    return new GribHandle(h);
}

bool GribHandle::hasKey(const char* key) const {
    return (grib_is_defined(handle_, key) != 0);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace grib
}  // namespace metkit
