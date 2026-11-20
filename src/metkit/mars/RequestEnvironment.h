// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @author Tiago Quintino
#pragma once

#include <map>
#include <mutex>
#include <optional>
#include <string>

#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

/// Encapsulates the environment to be sent to the mars-server.
/// This uses a mars request with 'environ' as the verb and each environment
/// variable is treated as keyword with its respective value.
class RequestEnvironment {
public:

    RequestEnvironment(const RequestEnvironment& other);

    /// Create the actual "mars-request"
    /// @return the request
    const MarsRequest& request() const { return *env_; }

    /// @brief Initialize the RequestEnvironment with a map of KEYWORD:VALUE(s).
    /// @param env, map of KEYWORD:VALUE (or KEYWORD:VALUE1/VALUE2/.../VALUEn) to add to the environment.
    static void initialize(const std::map<std::string, std::string>& env);

    /// @brief Update the RequestEnvironment with a map of KEYWORD:VALUE(s).
    /// @param env, map of KEYWORD:VALUE (or KEYWORD:VALUE1/VALUE2/.../VALUEn) to add to the environment.
    void update(const std::map<std::string, std::string>& env);

    /// Access instance of RequestEnvironment
    /// @return RequestEnvironment
    static const RequestEnvironment& instance();

private:

    RequestEnvironment() = default;

    static RequestEnvironment& inst();

    std::optional<MarsRequest> env_ = std::nullopt;
    std::recursive_mutex init_;
};

}  // namespace metkit::mars
