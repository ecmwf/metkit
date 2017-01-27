/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/grib/GribAccessor.h"

#include "grib_api.h"

#include "metkit/grib/GribHandle.h"

namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

static void check_error_code( const std::string& name, int err, bool quiet = false )
{
	if(err && !quiet) {
	   eckit::Log::error() << "GribAccessor(" << name << "): " << grib_get_error_message(err) << std::endl;
	}
}

void GribAccessorBase::grib_get_value(const GribHandle& h, const std::string& name, double& x, bool quiet) const
{
	x = 0;
	int err = grib_get_double(h.raw(), name.c_str(), &x);
	check_error_code(name,err,quiet);
}

void GribAccessorBase::grib_get_value(const GribHandle& h, const std::string& name, unsigned long& x, bool quiet) const
{
	long y = 0;
	int err = grib_get_long(h.raw(), name.c_str(), &y);
	check_error_code(name,err,quiet);
	x = y;
}

void GribAccessorBase::grib_get_value(const GribHandle& h, const std::string& name, long& x, bool quiet) const
{
	x = 0;
	int err = grib_get_long(h.raw(), name.c_str(), &x);
	check_error_code(name,err,quiet);
}

void GribAccessorBase::grib_get_value(const GribHandle& h, const std::string& name,  bool& x, bool quiet) const
{
   x = true;
   long xd = 0;
   int err = grib_get_long(h.raw(), name.c_str(), &xd);
   check_error_code(name,err,quiet);
   if (xd == 0) x = false;
}

void GribAccessorBase::grib_get_value(const GribHandle& h, const std::string& name, std::string& x, bool quiet) const
{
	char buf[1024];
	size_t s = sizeof(buf);
	buf[0] = 0;
	int err = grib_get_string(h.raw(), name.c_str(), buf, &s);
	check_error_code(name,err,quiet);
	x = buf;
}

void GribAccessorBase::grib_get_value(const GribHandle& h, const std::string& name, std::vector<long>& x, bool quiet) const
{
	int err = 0;
	size_t sz = 0;
	err = grib_get_size(h.raw(),name.c_str(),&sz); check_error_code(name,err,quiet);
	x.resize(sz);
	err = grib_get_long_array(h.raw(),name.c_str(),&x[0],&sz); check_error_code(name,err,quiet);
	ASSERT( x.size() == sz );
}

void GribAccessorBase::grib_get_value(const GribHandle& h, const std::string& name, std::vector<double>& x, bool quiet) const
{
	int err = 0;
	size_t sz = 0;
	err = grib_get_size(h.raw(),name.c_str(),&sz); check_error_code(name,err,quiet);
	x.resize(sz);
	err = grib_get_double_array(h.raw(),name.c_str(),&x[0],&sz); check_error_code(name,err,quiet);
	ASSERT( x.size() == sz );
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit
