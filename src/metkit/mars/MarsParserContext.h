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

#include <stddef.h>
#include <cstddef>
#include <iostream>

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class MarsParserContext {

public:

    MarsParserContext(std::size_t);
    virtual ~MarsParserContext() = default;

    virtual void info(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream& s, const MarsParserContext& r) {
        r.info(s);
        return s;
    }

private:

    std::size_t line_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
