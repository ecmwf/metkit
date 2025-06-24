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
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Main.h"

#include "metkit/mars/RequestEnvironment.h"

namespace {
struct PwDbScope {
    PwDbScope() { setpwent(); }
    ~PwDbScope() { endpwent(); }
};

std::string user() {
    // TODO(kkratz): This is not a great solution because there still could be concurrent access
    // to the pwdb. This stuff needs to move to eckit so that all calls to this are properly locked.
    static std::mutex mtx{};
    std::lock_guard lck{mtx};
    PwDbScope scope{};

    const auto uid = getuid();
    errno          = 0;
    do {
        const auto pw = getpwuid(uid);
        if (pw != nullptr) {
            return pw->pw_name;
        }
    } while (errno == EINTR);
    const auto error = errno;
    std::stringstream buf{};
    if (error == 0) {
        buf << "No user found for current uid " << uid << "!";
    }
    else {
        buf << "Error reading user name: " << strerror(error) << "!";
    }
    throw eckit::SeriousBug(buf.str());
}

}  // namespace

namespace metkit::mars {

RequestEnvironment::RequestEnvironment() : request_("environ") {
    request_.setValue("host", eckit::Main::hostname());
    request_.setValue("user", user());
    request_.setValue("pid", getpid());
    request_.setValue("client", "cpp");
}

void RequestEnvironment::update(const std::map<std::string, std::string>& env) {
    for (const auto& [k, v] : env) {
        request_.setValue(k, v);
    }
}

RequestEnvironment RequestEnvironment::make() {
    static RequestEnvironment e;
    return e;
}

RequestEnvironment RequestEnvironment::instance() {
    return make();
}

}  // namespace metkit::mars
