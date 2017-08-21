/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date Dec 2015

#include "grib_api.h"

#include "eckit/io/Buffer.h"
#include "eckit/io/BufferedHandle.h"
#include "eckit/io/MoverHandle.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"

#include "metkit/grib/MetFile.h"
#include "eckit/io/CircularBuffer.h"
#include "eckit/io/ResizableBuffer.h"



namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

static long readcb(void *data, void *buffer, long len)
{
    eckit::DataHandle *handle = reinterpret_cast<eckit::DataHandle*>(data);
    return handle->read(buffer, len);
}

MetFile::MetFile(const eckit::PathName& path, bool buffered):
    handle_(
        buffered
        ? new eckit::BufferedHandle(path.fileHandle(), 64 * 1024 * 1024)
        : path.fileHandle()
    )
{
    handle_->openForRead();
}

MetFile::MetFile( eckit::DataHandle& dh ):
    handle_(new eckit::BufferedHandle( dh, 64 * 1024 * 1024))
{
    handle_->openForRead();
}

eckit::Offset MetFile::position()
{
    return handle_->position();
}

void MetFile::rewind()
{
    handle_->rewind();
}

void MetFile::seek(const eckit::Offset& where)
{
    handle_->seek(where);
}

MetFile::~MetFile()
{
    handle_->close();
}

long MetFile::read(eckit::Buffer& buffer)
{
    size_t len = buffer.size();
    int e    = wmo_read_any_from_stream(handle_.get(), &readcb, buffer, &len);

    if (e == GRIB_SUCCESS)  return len;
    if (e == GRIB_END_OF_FILE) return 0;

    throw eckit::ReadError("in MetFile::read");
}

namespace {
class AutoFree {
    void *p_;
public:
    AutoFree(void* p): p_(p) {}
    ~AutoFree() { if (p_) ::free(p_); }
};
}

long MetFile::read(eckit::CircularBuffer& buffer)
{
    int e = 0;
    size_t len;
    void * p = wmo_read_any_from_stream_malloc(handle_.get(), &readcb, &len, &e);
    AutoFree free(p);

    if (e == GRIB_SUCCESS)  {
        ASSERT(p);
        buffer.write(p, len);
        return len;
    }

    if (e == GRIB_END_OF_FILE) {
        return 0;
    }

    throw eckit::ReadError("in MetFile::read");
}


long MetFile::read(eckit::ResizableBuffer& buffer)
{
    int e = 0;
    size_t len;
    void * p = wmo_read_any_from_stream_malloc(handle_.get(), &readcb, &len, &e);
    AutoFree free(p);

    if (e == GRIB_SUCCESS)  {
        ASSERT(p);
        buffer.resize(std::max(buffer.size(), len));
        ::memcpy(buffer, p, len);
        return len;
    }

    if (e == GRIB_END_OF_FILE) {
        return 0;
    }

    throw eckit::ReadError("in MetFile::read");
}


long MetFile::readSome(eckit::Buffer& buffer)
{
    size_t len = buffer.size();
    int e    = wmo_read_any_from_stream(handle_.get(), &readcb, buffer, &len);

    if (e == GRIB_SUCCESS || e == GRIB_BUFFER_TOO_SMALL)  return len;
    if (e == GRIB_END_OF_FILE)           return 0;

    throw eckit::ReadError("in MetFile::readSome");
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit


