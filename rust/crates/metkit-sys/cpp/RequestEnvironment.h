// metkit RequestEnvironment bridge — wraps the singleton
// `metkit::mars::RequestEnvironment`.
#pragma once

#include "MarsRequest.h"

#include "rust/cxx.h"

#include <memory>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Wraps the singleton `metkit::mars::RequestEnvironment`. All members are
/// static — the underlying C++ type is itself a process-wide singleton.
class RequestEnvironmentWrapper {
public:

    /// Initialise the global request environment from a list of key/value pairs.
    static void initialise(rust::Vec<rust::String> keys, rust::Vec<rust::String> values);

    /// Return the current environment's MARS request.
    static std::unique_ptr<MarsRequestWrapper> request();
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge
