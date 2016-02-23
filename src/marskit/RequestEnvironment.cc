/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File RequestEnvironment.cc
// Baudouin Raoult - (c) ECMWF Feb 12

#include <unistd.h>

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "marskit/RequestEnvironment.h"

using namespace eckit;

namespace marskit {

static Mutex local_mutex;
RequestEnvironment::RequestEnvironment():
    request_("environ")
{
    // TODO....
    request_.setValue("user","max");
    request_.setValue("host","localhost");
    request_.setValue("pid",long(::getpid()));
}

RequestEnvironment::~RequestEnvironment()
{
}


void RequestEnvironment::print(std::ostream&) const
{
}

RequestEnvironment& RequestEnvironment::instance()
{
    AutoLock<Mutex> lock(local_mutex);
    {
        static RequestEnvironment e;
        return e;
    }
}

}
