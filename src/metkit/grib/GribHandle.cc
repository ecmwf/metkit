/*
 * (C) Copyright 1996-2017 ECMWF.
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

#include "metkit/grib/GribMetaData.h"
#include "metkit/grib/GribAccessor.h"
#include "metkit/grib/GribDataBlob.h"
#include "metkit/grib/GribMetaData.h"

using namespace std;


namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

void grib_call(int code, const char *msg, const eckit::CodeLocation& where)
{
	if(code)
	{
        std::ostringstream os;
        os << msg << " : " << grib_get_error_message(code);
        throw eckit::Exception(os.str(), where);
	}
}

//----------------------------------------------------------------------------------------------------------------------

GribHandle::GribHandle(const eckit::PathName& path) :
    handle_(NULL),
    owned_(true)
{
	eckit::StdFile f(path);

	int err = 0;
	grib_handle* h = grib_handle_new_from_file(0,f,&err);
	if(err != 0)
	{
        std::ostringstream os;
        os << "GribHandle() failed to build from path " << path;
        throw eckit::Exception(os.str(), Here());
	}

	ASSERT(h);
	handle_ = h;
}

GribHandle::GribHandle(grib_handle* h) :
    handle_(NULL),
    owned_(true)
{
	ASSERT(h);
    handle_ = h;
}

GribHandle::GribHandle(grib_handle& h) :
    handle_(&h),
    owned_(false)
{
}

GribHandle::GribHandle(const eckit::Buffer& buffer, bool copy)
	: handle_(NULL)
{
	const char *message = buffer;
	ASSERT(strncmp(message,"GRIB", 4) == 0);

	grib_handle *h = 0;

	if(copy)
	{
		h = grib_handle_new_from_message_copy(0,const_cast<char*>(message),buffer.size());
	}
	else
	{
		h = grib_handle_new_from_message(0,const_cast<char*>(message),buffer.size());
	}

	ASSERT(h);
	handle_ = h;
}

GribHandle::~GribHandle()
{
    if( handle_ && owned_ )
	{
		GRIB_CALL( grib_handle_delete(handle_) );
		handle_ = 0;
    }
}

GribDataBlob* GribHandle::message() const
{
    size_t length;
    const void* message;
    GRIB_CALL( grib_get_message(handle_, &message, &length) );
    return new GribDataBlob(message, length);
}

std::string GribHandle::gridType() const
{
	return GribAccessor<std::string>("gridType")(*this);
}

std::string GribHandle::geographyHash() const
{
   // The following key is edition independent
   return GribAccessor<std::string>("md5GridSection")(*this);
}

long GribHandle::edition() const
{
	return GribAccessor<long>("edition")(*this);
}

size_t GribHandle::getDataValuesSize() const
{
    size_t count = 0;
	GRIB_CALL(grib_get_size(raw(), "values", &count));
    return count;
}

void GribHandle::getDataValues(double* values, const size_t& count) const
{
    ASSERT(values);
    size_t n = count;
	GRIB_CALL(grib_get_double_array(raw(),"values",values,&n));
	ASSERT(n == count);
}

double* GribHandle::getDataValues(size_t& count) const
{
    count = getDataValuesSize();

    double* values = new double[count];
    getDataValues(values,count);
    return values;
}

void GribHandle::setDataValues(const double *values, size_t count)
{
    ASSERT(values);
	GRIB_CALL(grib_set_double_array(raw(),"values",values,count));
}

void GribHandle::write( eckit::DataHandle& handle )
{
	const void* message = NULL;
	size_t length = 0;

	GRIB_CALL(grib_get_message(raw(), &message, &length));

	ASSERT( message );
	ASSERT( length );

    ASSERT( length = long(length) && handle.write(message, length) == long(length) );
}

size_t GribHandle::write( eckit::Buffer& buff )
{
    size_t len = buff.size();
	GRIB_CALL( grib_get_message_copy( raw(), buff, &len )); // will issue error if buffer too small
	return len;
}

double GribHandle::latitudeOfFirstGridPointInDegrees() const
{
	return GribAccessor<double>("latitudeOfFirstGridPointInDegrees")(*this);
}

double GribHandle::longitudeOfFirstGridPointInDegrees() const
{
	return GribAccessor<double>("longitudeOfFirstGridPointInDegrees")(*this);
}

double GribHandle::latitudeOfLastGridPointInDegrees() const
{
	return GribAccessor<double>("latitudeOfLastGridPointInDegrees")(*this);
}

double GribHandle::longitudeOfLastGridPointInDegrees() const
{
	return GribAccessor<double>("longitudeOfLastGridPointInDegrees")(*this);
}

GribHandle* GribHandle::clone() const
{
	grib_handle* h = grib_handle_clone(raw());
    if(!h)
        throw eckit::WriteError( std::string("failed to clone output grib") );

	return new GribHandle(h);
}

string GribHandle::shortName() const
{
	return GribAccessor<std::string>("shortName")(*this);
}

size_t GribHandle::numberOfPoints() const
{
	return GribAccessor<long>("numberOfDataPoints")(*this);
}

bool GribHandle::hasKey(const char* key) const {
    return (grib_is_defined(handle_,key) != 0);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit
