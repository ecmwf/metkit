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
#include "metkit/MarsExpension.h"

using namespace eckit;
using namespace metkit;

//----------------------------------------------------------------------------------------------------------------------

class ParseRequest : public Tool {
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
        std::vector<PathName> files;
        std::vector<PathName> directories;

        path.children(files, directories);

        for (std::vector<PathName>::const_iterator j = files.begin(); j != files.end(); ++j) {
            process(*j);
        }

        for (std::vector<PathName>::const_iterator j = directories.begin(); j != directories.end(); ++j) {
            process(*j);
        }
        return;
    }


    std::cout << "============= " << path << std::endl;
    std::ifstream in(path.asString().c_str());
    MarsParser parser(in);
    MarsExpension expand;

    std::vector<MarsRequest> v = expand(parser.parse());

    for (std::vector<MarsRequest>::const_iterator j = v.begin(); j != v.end(); ++j) {
        std::cout << *j << std::endl;
    }

    class Print : public FlattenCallback {
        virtual void operator()(const MarsRequest& request)  {
            std::cout << request << std::endl;
        }

    };

    class Filter : public FlattenFilter {
        virtual bool operator()(const std::string& keyword,
                                std::vector<std::string>& values,
                                const MarsRequest& request)  {

            if (keyword == "levelist") {
                values.erase(
                    std::remove_if(values.begin(),
                                   values.end(),
                                   std::bind1st(std::equal_to<std::string>(), "850")),
                    values.end());
            }

            if (keyword == "step") {
                values.erase(
                    std::remove_if(values.begin(),
                                   values.end(),
                                   std::bind1st(std::equal_to<std::string>(), "216-240")),
                    values.end());
            }
            return true;
        }

    };

    Print cb;
    Filter filter;

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
