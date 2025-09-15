/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#include "metkit/mars/RequestEnvironment.h"

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <map>
#include <string>

#include "eckit/runtime/Main.h"
#include "eckit/system/SystemInfo.h"


namespace metkit::mars {

RequestEnvironment::RequestEnvironment() : request_("environ") {
    request_.setValue("host", eckit::Main::hostname());
    request_.setValue("user", eckit::system::SystemInfo::instance().userName());
    request_.setValue("pid", getpid());
    request_.setValue("client", "cpp");
}

void RequestEnvironment::update(const std::map<std::string, std::string>& env) {
    for (const auto& [k, v] : env) {
        request_.setValue(k, v);
    }
}

const RequestEnvironment& RequestEnvironment::instance() {
    static RequestEnvironment e;
    return e;
}

}  // namespace metkit::mars
