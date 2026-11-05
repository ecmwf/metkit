// metkit RequestEnvironment bridge — implementation.

#include "metkit_exceptions.h"

#include "RequestEnvironment.h"

#include "metkit/mars/RequestEnvironment.h"

#include <map>
#include <string>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

void RequestEnvironmentWrapper::initialise(rust::Vec<rust::String> keys, rust::Vec<rust::String> values) {
    std::map<std::string, std::string> env;
    for (size_t i = 0; i < keys.size() && i < values.size(); ++i) {
        env[std::string(keys[i])] = std::string(values[i]);
    }
    metkit::mars::RequestEnvironment::initialize(env);
}

std::unique_ptr<MarsRequestWrapper> RequestEnvironmentWrapper::request() {
    const auto& env = metkit::mars::RequestEnvironment::instance();
    return std::make_unique<MarsRequestWrapper>(env.request());
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
