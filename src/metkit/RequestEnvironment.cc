/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "metkit/RequestEnvironment.h"

#include <sys/types.h>
#include <pwd.h>



namespace metkit {

static eckit::Mutex local_mutex;

RequestEnvironment::RequestEnvironment():
    request_("environ")
{
	char buf[1024];
	if(gethostname(buf,sizeof(buf)) != 0)
		throw eckit::SeriousBug("Cannot establish current hostname");

    request_.setValue("host", std::string(buf));


	struct passwd *pw;
	setpwent();

	if((pw = getpwuid(getuid())) == NULL)
		throw eckit::SeriousBug("Cannot establish current user");

    request_.setValue("user", std::string(pw->pw_name));

	endpwent();


   request_.setValue("pid",long(::getpid()));
   request_.setValue("client", "cpp");


   // Tell server that we use paramid, e.g. 130 instead of 130.128
   request_.setValue("use-paramid", true);


}

RequestEnvironment::~RequestEnvironment()
{
}


void RequestEnvironment::print(std::ostream&) const
{
}

RequestEnvironment& RequestEnvironment::instance()
{
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    {
        static RequestEnvironment e;
        return e;
    }
}

}
