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
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Option.h"
#include "eckit/runtime/Tool.h"
#include "eckit/log/Log.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/grib/MetFile.h"
#include "metkit/bufr/BufrToRequest.h"
#include "metkit/bufr/BufrHandle.h"

using namespace metkit;
using namespace metkit::mars;

//----------------------------------------------------------------------------------------------------------------------

class BufrToMarsRequest;
static BufrToMarsRequest* instance_ = nullptr;

class BufrToMarsRequest : public eckit::Tool {
public:
    virtual void usage(const std::string& tool) const {
        eckit::Log::info() << std::endl
                           << "Usage: " << tool << " <path1> [path2] [...]" << std::endl;
    }

    virtual int numberOfPositionalArguments() const { return -1; }
    virtual int minimumPositionalArguments() const { return 1; }

    virtual void run();

    BufrToMarsRequest(int argc, char** argv) : eckit::Tool(argc, argv) {
        ASSERT(instance_ == nullptr);
        instance_ = this;
    }

protected:  // members
    std::vector<eckit::option::Option*> options_;

};

static void usage(const std::string& tool) {
    ASSERT(instance_);
    instance_->usage(tool);
}
void BufrToMarsRequest::run() {

    eckit::option::CmdArgs args(&::usage,
                                options_,
                                numberOfPositionalArguments(),
                                minimumPositionalArguments());

//    metkit::mars::MarsRequest onereq("BUFR");

    static size_t bufferSize = eckit::Resource<size_t>("BufferSize", 64 * 1024 * 1024);
    eckit::Buffer buffer(bufferSize);
    long len = 0;

    size_t messages = 0;
    for (size_t i = 0; i < args.count(); i++) {

        eckit::PathName path(args(i));

        eckit::Log::info() << "Processing " << path << std::endl;

        grib::MetFile file(path);
        while ((len = file.readSome(buffer)) != 0) {

            metkit::mars::MarsRequest req("BUFR");

            metkit::bufr::BufrToRequest::messageToRequest(buffer, size_t(len), req);

//            eckit::Log::info() << req << std::endl;

            ++messages;

//            onereq.merge(req);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    BufrToMarsRequest tool(argc, argv);
    return tool.start();
}
