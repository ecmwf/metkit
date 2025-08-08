/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Mar 98

#pragma once

#include "eckit/io/Length.h"
#include "eckit/types/Types.h"

namespace eckit {
class Stream;
}
namespace eckit {
class JSON;
}

namespace eckit {
class Metrics;
}

namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

class Cost {
public:
    Cost() { reset(); }

    void reset();

    eckit::Ordinal layout_;

    eckit::Length onLine_;
    eckit::Length offLine_;

    eckit::Ordinal tapes_;
    eckit::Ordinal disks_;
    eckit::Ordinal unavailable_;
    eckit::Ordinal offsite_;

    std::set<std::string> damaged_;

    eckit::Ordinal onLineFields_;
    eckit::Ordinal offLineFields_;

    std::set<std::string> media_;
    std::set<std::string> nodes_;
    std::set<std::string> libraries_;

    Cost& operator+=(const Cost& other);

    time_t updated_;

    void print(std::ostream&) const;

    void json(eckit::JSON&) const;

    void collectMetrics() const;


    friend std::ostream& operator<<(std::ostream& s, const Cost& c) {
        c.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Cost& c) {
        c.json(s);
        return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream&, const Cost&);

    friend eckit::Stream& operator>>(eckit::Stream&, Cost&);
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules
