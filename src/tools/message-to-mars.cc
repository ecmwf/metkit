/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/Resource.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/Offset.h"
#include "eckit/runtime/Tool.h"


#include "metkit/codes/GribToRequest.h"
#include "metkit/mars/MarsRequest.h"

using namespace metkit;
using metkit::grib::MetFile;

//----------------------------------------------------------------------------------------------------------------------

class Grib2Request : public eckit::Tool {
public:

    Grib2Request(int argc, char** argv) : Tool(argc, argv) {

        path_ = eckit::Resource<std::string>("-in", "input.grib");  ///< @todo Move to use Option
    }

    virtual ~Grib2Request() {}

    virtual void run();

private:  // members

    eckit::PathName path_;
};

void Grib2Request::run() {
    eckit::Log::debug() << "Opening GRIB file : " << path_ << std::endl;

    eckit::Buffer buffer(MetFile::gribBufferSize());

    long len = 0;

    grib::MetFile file(path_);

    metkit::mars::MarsRequest onereq("GRIB");

    size_t nMsg = 0;
    while ((len = file.readSome(buffer)) != 0) {
        metkit::mars::MarsRequest req("GRIB");

        grib::GribToRequest::gribToRequest(buffer, len, req);

        // eckit::Log::info() << req << std::endl;

        ++nMsg;

        onereq.merge(req);
    }

    eckit::Log::info() << onereq << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    Grib2Request tool(argc, argv);
    return tool.start();
}
