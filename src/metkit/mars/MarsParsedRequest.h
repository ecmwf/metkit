// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class MarsParsedRequest : public MarsRequest {
public:

    MarsParsedRequest(const std::string& verb, size_t line);
    virtual ~MarsParsedRequest() = default;

    virtual void info(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream& s, const MarsParsedRequest& r) {
        r.info(s);
        return s;
    }

private:

    std::size_t line_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
