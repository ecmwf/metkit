/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/Resource.h"
#include "eckit/runtime/Tool.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/Offset.h"

#include "metkit/grib/EmosFile.h"

#include "metkit/grib/GribToRequest.h"
#include "metkit/MarsRequest.h"

using namespace eckit;
using namespace metkit;

//----------------------------------------------------------------------------------------------------------------------

class Grib2Request : public Tool {
public:

    Grib2Request(int argc,char **argv) :
        Tool(argc,argv) {

        path_ = eckit::Resource<std::string>("-in","input.grib"); ///< @todo Move to use Option

    }

    virtual ~Grib2Request() {}

    virtual void run();

private: // members

     eckit::PathName path_;
};

void Grib2Request::run()
{
    Log::debug() << "Opening GRIB file : " << path_ << std::endl;

    static long gribBufferSize = eckit::Resource<long>("gribBufferSize", 64*1024*1024);

    Buffer buffer(gribBufferSize);

    long len = 0;

    grib::EmosFile file( path_ );

    metkit::MarsRequest onereq("GRIB");

    size_t nMsg = 0;
    while( (len = file.readSome(buffer)) != 0 )
    {
        metkit::MarsRequest req("GRIB");

        grib::GribToRequest::gribToRequest(buffer, len, req);

        // Log::info() << req << std::endl;

        ++nMsg;

        onereq.merge(req);
    }

    Log::info() << onereq << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    Grib2Request tool(argc,argv);
    return tool.start();
}
