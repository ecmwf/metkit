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
#include "eckit/utils/Tokenizer.h"


namespace metkit::mars {

RequestEnvironment::RequestEnvironment(const RequestEnvironment& other) : env_(other.env_) {}

void RequestEnvironment::update(const std::map<std::string, std::string>& env) {
    // Split string on '/' and ignore empty splits
    const auto split = [](const std::string& str) -> std::vector<std::string> {
        std::vector<std::string> result;
        eckit::Tokenizer parse("/");
        parse(str, result);
        return result;
    };
    for (const auto& [k, v] : env) {
        env_->values(k, split(v));
    }
}

void RequestEnvironment::initialize(const std::map<std::string, std::string>& env) {

    RequestEnvironment& re = inst();
    std::lock_guard<std::recursive_mutex> lock(re.init_);

    re.env_.emplace("environ");
    re.env_->setValue("host", eckit::Main::hostname());
    re.env_->setValue("user", eckit::system::SystemInfo::instance().userName());
    re.env_->setValue("pid", getpid());
    re.env_->setValue("client", "unknown");

    re.update(env);
}

RequestEnvironment& RequestEnvironment::inst() {
    static RequestEnvironment e;
    return e;
}

const RequestEnvironment& RequestEnvironment::instance() {
    auto& re = RequestEnvironment::inst();
    std::lock_guard<std::recursive_mutex> lock(re.init_);
    if (!re.env_) {
        throw eckit::SeriousBug("RequestEnvironment not initialized");
    }
    return re;
}

}  // namespace metkit::mars
