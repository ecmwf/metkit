/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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
