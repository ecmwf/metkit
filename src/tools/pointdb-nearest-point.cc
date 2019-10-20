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
#include "eckit/config/Resource.h"
#include "metkit/pointdb/SimpleGribDataSource.h"


using namespace metkit;
using namespace metkit::pointdb;

//----------------------------------------------------------------------------------------------------------------------



class NearestPoint : public eckit::Tool {

public:

    static void usage(const std::string &tool) {
        eckit::Log::info() << std::endl
                           << "Usage: " << tool << " lat lon path"
                           << std::endl;
    }

    virtual void run();

    NearestPoint(int argc, char **argv) : eckit::Tool(argc, argv) {

    }

protected: // members

    std::vector<eckit::option::Option *> options_;

};


void NearestPoint::run() {

    eckit::option::CmdArgs args(&usage, options_, 1);

    SimpleGribDataSource s(args(0));
    std::cout << s << std::endl;

    std::cout << s.extract(50, -1) << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    NearestPoint tool(argc,argv);
    return tool.start();
}
