/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "eckit/runtime/Main.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "metkit/mars/RequestEnvironment.h"


namespace metkit {
namespace mars {

static eckit::Mutex local_mutex;

RequestEnvironment::RequestEnvironment() : request_("environ") {
    std::string host = eckit::Main::hostname();
    request_.setValue("host", host);

    struct passwd* pw;
    setpwent();

    if ((pw = getpwuid(getuid())) == NULL) {
        throw eckit::SeriousBug("Cannot establish current user");
    }

    request_.setValue("user", std::string(pw->pw_name));

    endpwent();


    request_.setValue("pid", long(::getpid()));
    request_.setValue("client", "cpp");
}

RequestEnvironment::~RequestEnvironment() {}


void RequestEnvironment::print(std::ostream&) const {}

RequestEnvironment& RequestEnvironment::instance() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    {
        static RequestEnvironment e;
        return e;
    }
}

}  // namespace mars
}  // namespace metkit
