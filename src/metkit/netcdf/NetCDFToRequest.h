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

/// @date July 2017

#ifndef metkit_netcdf_metkit_netcdf_NetCDFToRequest
#define metkit_netcdf_metkit_netcdf_NetCDFToRequest

#include "eckit/filesystem/PathName.h"

namespace metkit {

namespace metkit{
namespace netcdf{

class MarsRequest;

namespace netcdf {

//----------------------------------------------------------------------------------------------------------------------

/// Utility class to build MarsRequest from a NetCDF

namespace metkit{
namespace netcdf{

class NetCDFToRequest {

public: // methods

    static void handleToRequest(const eckit::PathName& path, MarsRequest& req);

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace netcdf
} // namespace metkit

}
}
#endif
