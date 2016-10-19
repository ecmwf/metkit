/*
 * (C) Copyright 1996-2015 ECMWF.
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
#include "metkit/MarsParser.h"

using namespace eckit;
using namespace metkit;

//----------------------------------------------------------------------------------------------------------------------

class Grib2Request : public Tool {
public:

    Grib2Request(int argc,char **argv) :
        Tool(argc,argv) {

        path_ = eckit::Resource<std::string>("-in","/Users/baudouin/Dropbox/B1.diss"); ///< @todo Move to use Option

    }

    virtual ~Grib2Request() {}

    virtual void run();

private: // members

     eckit::PathName path_;
};

void Grib2Request::run()
{
    std::ifstream in(path_);
    MarsParser parser(in);
    auto v = parser.parse();
    for(auto j = v.begin(); j != v.end(); ++j) {
        std::cout << *j << std::endl;
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    Grib2Request tool(argc,argv);
    return tool.start();
}
