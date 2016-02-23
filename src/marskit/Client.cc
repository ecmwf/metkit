/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>

#include "marskit/DHSProtocol.h"
#include "marskit/MarsRequestHandle.h"
#include "eckit/ecml/parser/RequestParser.h"
#include "marskit/Client.h"


#include "eckit/runtime/Tool.h"
#include "eckit/io/FileHandle.h"
#include "eckit/runtime/Context.h"

#include "marskit/MarsRequest.h"

#include "eckit/ecml/core/Interpreter.h"

using namespace std;

eckit::ExecutionContext& Client::executionContext() { return context_; }
Client::Client(int argc, char** argv) : eckit::Tool(argc,argv) {}

void Client::run()
{
    int argc (eckit::Context::instance().argc());
    if (argc < 2)
        throw eckit::UserError("Command line required (name(s) of file(s) with MARS request)");

    eckit::Values r(0);
    for (size_t i(1); i < argc; ++i)
    {
        const eckit::PathName requestFile (eckit::Context::instance().argv(i));

        eckit::Log::info() << "Client: Processing file " << requestFile << endl;

        context_.executeScriptFile(requestFile);
    }
}

