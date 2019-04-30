/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/io/FileHandle.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Option.h"
#include "eckit/runtime/Tool.h"

#include "metkit/odb/OdbToRequest.h"


using namespace metkit;
using namespace eckit;

//----------------------------------------------------------------------------------------------------------------------

class OdbToRequestTool : public Tool {
public:

    OdbToRequestTool(int argc, char **argv) : Tool(argc, argv) {
    }

    virtual ~OdbToRequestTool() {}

private: // methods

    virtual void run();

private: // members

    std::vector<option::Option*> options_;

    PathName path_;


};

static void usage(const std::string& tool) {
    Log::error() << "Usage: " << std::endl;
    Log::error() << "    " << tool << " <odb2 file> [<output file>]" << std::endl;
}

void OdbToRequestTool::run() {

    option::CmdArgs args(&usage, options_, -1, 1);

    PathName inFile(args(0));

    FileHandle dh(inFile);
    dh.openForRead();

    for (const MarsRequest& request : odb::OdbToRequest(false, true).odbToRequest(dh)) {
        eckit::Log::info() << request << std::endl;
    }

}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    OdbToRequestTool tool(argc, argv);
    return tool.start();
}
