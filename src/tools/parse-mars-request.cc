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

#include "metkit/grib/MetFile.h"

#include "metkit/grib/GribToRequest.h"
#include "metkit/MarsRequest.h"
#include "metkit/MarsParser.h"
#include "metkit/MarsExpension.h"


using namespace metkit;

//----------------------------------------------------------------------------------------------------------------------

class ParseRequest : public eckit::Tool {
public:

    ParseRequest(int argc, char **argv) :
        Tool(argc, argv) {

        // path_ = eckit::Resource<std::string>("-in", "/Users/baudouin/Dropbox/diss/EC/EC1/req/curr/C1"); ///< @todo Move to use Option
        // path_ = eckit::Resource<std::string>("-in", "/Users/baudouin/Dropbox/diss/XS/NA4/req/curr/N2");
        // path_ = eckit::Resource<std::string>("-in", "/Users/baudouin/Dropbox/diss/CZ/CZM/req/curr/CJ");
        // path_ = eckit::Resource<std::string>("-in", "/Users/baudouin/Dropbox/diss/FR/FRA/req/curr/FR");
        path_ = eckit::Resource<std::string>("-in", "/Users/baudouin/Dropbox/diss");

    }

    virtual ~ParseRequest() {}

private: // methods

    virtual void run();
    void process(const eckit::PathName& path);

private: // members

    eckit::PathName path_;


};

void ParseRequest::run() {
    process(path_);
}

void ParseRequest::process(const eckit::PathName& path)
{

    if (path.isDir()) {
        std::vector<eckit::PathName> files;
        std::vector<eckit::PathName> directories;

        path.children(files, directories);

        std::sort(files.begin(), files.end());
        std::sort(directories.begin(), directories.end());

        for (std::vector<eckit::PathName>::const_iterator j = files.begin(); j != files.end(); ++j) {
            process(*j);
        }

        for (std::vector<eckit::PathName>::const_iterator j = directories.begin(); j != directories.end(); ++j) {
            process(*j);
        }
        return;
    }


    std::cout << "==========> Parsing : " << path << std::endl;

    std::ifstream in(path.asString().c_str());
    MarsParser parser(in);

    bool inherit = true;
    MarsExpension expand(inherit);

    std::vector<MarsRequest> p = parser.parse();
    for (std::vector<MarsRequest>::const_iterator j = p.begin(); j != p.end(); ++j) {
        (*j).dump(std::cout);
    }

    std::cout << "----------> Expanding ... " << std::endl;

    std::vector<MarsRequest> v = expand.expand(p);

    for (std::vector<MarsRequest>::const_iterator j = v.begin(); j != v.end(); ++j) {
        (*j).dump(std::cout);
    }

    class Print : public FlattenCallback {
        virtual void operator()(const MarsRequest& request)  {
            std::cout << request << std::endl;
        }

    };


    Print cb;

    // for (std::vector<MarsRequest>::const_iterator j = v.begin(); j != v.end(); ++j) {
    //     expand.flatten(*j, cb, filter);
    // }

}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    ParseRequest tool(argc, argv);
    return tool.start();
}
