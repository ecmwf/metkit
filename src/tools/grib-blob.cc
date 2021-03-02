/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/option/Option.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/runtime/Tool.h"
#include "eckit/io/Buffer.h"
#include "eckit/config/Resource.h"

#include "metkit/codes/GribMetaData.h"

using namespace metkit;

//----------------------------------------------------------------------------------------------------------------------

class GribBlob;

static GribBlob* instance_ = 0;

class GribBlob : public eckit::Tool {

public:

    virtual void usage(const std::string &tool) const {
        eckit::Log::info() << std::endl
                           << "Usage: " << tool << " <path1> [path2] ..."
                           << std::endl;
    }

    virtual int numberOfPositionalArguments() const { return -1; }
    virtual int minimumPositionalArguments() const { return 1; }

    virtual void run();

    GribBlob(int argc, char **argv) : eckit::Tool(argc, argv) {
        ASSERT(instance_ == 0);
        instance_ = this;
    }

protected: // members

    std::vector<eckit::option::Option *> options_;

};

static void usage(const std::string &tool) {
    ASSERT(instance_);
    instance_->usage(tool);
}

void GribBlob::run() {

    eckit::option::CmdArgs args(&::usage, options_, numberOfPositionalArguments(), minimumPositionalArguments());


    eckit::Buffer buffer(grib::MetFile::gribBufferSize());
    long len = 0;

    for (size_t i = 0; i < args.count(); i++) {

        eckit::PathName path(args(i));
        std::cout << "Processing " << path << std::endl;

        grib::MetFile file(path);

        size_t nMsg = 0;
        while( (len = file.readSome(buffer)) != 0 )
        {
            metkit::grib::GribMetaData grib(buffer, len);
            ++nMsg;

            eckit::Log::info() << nMsg << " " << grib << std::endl;
        }
    }

}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    GribBlob tool(argc,argv);
    return tool.start();
}
