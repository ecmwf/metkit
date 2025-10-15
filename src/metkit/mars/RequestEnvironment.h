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

    /// Access instance of RequestEnvironment
    /// @return RequestEnvironment
    static const RequestEnvironment& instance();

private:

    RequestEnvironment();

    MarsRequest request_;
};

}  // namespace metkit::mars
