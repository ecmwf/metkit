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

#include "eckit/io/FileHandle.h"

#include "eckit/ecml/parser/Request.h"
#include "eckit/ecml/parser/RequestParser.h"
#include "eckit/ecml/core/RequestHandler.h"

#include "marskit/MarsRequestHandle.h"
#include "marskit/MarsHandleFactory.h"
#include "marskit/DHSProtocol.h"

using namespace eckit;
using namespace std;

namespace marskit {

MarsHandleFactory::MarsHandleFactory()
: DataHandleFactory("mars")
{}

DataHandle* MarsHandleFactory::makeHandle(const string& r) const
{
    Log::info() << "MarsHandleFactory::makeHandle: parsing " << r << endl;

    Request requests (eckit::RequestParser::parse(r));

    Log::info() << "MarsHandleFactory::makeHandle: requests = " << requests << endl;

    Request request(requests->value());

    Log::info() << "MarsHandleFactory::makeHandle: request = " << request << endl;

    if (requests->rest())
        Log::warning() << "MarsHandleFactory: Only " << request << " used, skipped rest of " << requests << endl;

    string host(RequestHandler::database(request));
    long port(RequestHandler::port(request));
    return new MarsRequestHandle(request, new DHSProtocol(host, host, port));
}

static MarsHandleFactory marsHandleFactory;

}
