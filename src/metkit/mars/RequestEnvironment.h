/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
#pragma once
#include "metkit/mars/MarsRequest.h"

#include <map>
#include <string>

namespace metkit::mars {

/// Encapsulates the environment to be sent to the mars-server.
/// This uses a mars request with 'environ' as the verb and each environment
/// variable is treated as keyword with its respective value.
class RequestEnvironment {
public:

    /// Create the actual "mars-request"
    /// @return the request
    const MarsRequest& request() const { return request_; }

    /// Update with a map of VARIABLE:VALUE.
    /// Already know keys will be overwritten.
    /// @param env, map of VARIABLE:VALUE to add to the request.
    void update(const std::map<std::string, std::string>& env);

    /// Create an instance of RequestEnvironment - DEPRECATED
    /// @note Originally this returned a reference to the global
    /// instance but with the introduction of 'update' the object now
    /// allows mutation. To prevent mutation of global state a copy is now
    /// returned. Since this is different to how singletons are generally used
    /// 'RequestEnvironemnt::make()` is introduced instead to clearly distinguish
    /// it.
    [[deprecated("Use RequestEnvironment::make() instead")]]
    static RequestEnvironment instance();

    /// Create a new RequestEnvironment.
    /// Contains the following variables:
    /// [host, user, client, pid]
    /// @return a new RequestEnvironment
    static RequestEnvironment make();

private:

    RequestEnvironment();

    MarsRequest request_;
};

}  // namespace metkit::mars
