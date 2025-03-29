/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File StepRange.h
// Baudouin Raoult - ECMWF Dec 97

#ifndef metkit_StepRange_H
#define metkit_StepRange_H

#include "eckit/persist/Bless.h"
#include "eckit/types/Time.h"
#include "eckit/types/Types.h"

namespace eckit {
class DumpLoad;
}

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class StepRange {
public:

    // -- Exceptions
    // None

    // -- Contructors

    StepRange(const std::string&);

    explicit StepRange(eckit::Time from = eckit::Time(0), eckit::Time to = eckit::Time(0)) :
        from_(from / 3600.), to_(to / 3600.) {

        if (from != eckit::Time(0) && to == eckit::Time(0)) {
            to_ = from_;
        }
    }

    explicit StepRange(double from, double to = 0) :
        StepRange(eckit::Time(from * 3600, true), eckit::Time(to * 3600, true)) {}


#include "metkit/mars/StepRange.b"

    // -- Destructor

    ~StepRange() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    operator std::string() const;

    StepRange& operator+=(const eckit::Time& step) {
        from_ += step / 3600.;
        to_ += step / 3600.;
        return *this;
    }

    StepRange& operator-=(const eckit::Time& step) {
        from_ -= step / 3600.;
        to_ -= step / 3600.;
        return *this;
    }

    bool operator==(const StepRange& other) const { return from_ == other.from_ && to_ == other.to_; }

    bool operator!=(const StepRange& other) const { return from_ != other.from_ || to_ != other.to_; }

    bool operator<(const StepRange& other) const {
        return (from_ != other.from_) ? (from_ < other.from_) : (to_ < other.to_);
    }

    bool operator<=(const StepRange& other) const {
        return (from_ != other.from_) ? (from_ <= other.from_) : (to_ <= other.to_);
    }

    bool operator>(const StepRange& other) const { return other < *this; }

    // -- Methods

    double from() const { return from_; }
    double to() const { return to_; }

    void dump(eckit::DumpLoad&) const;
    void load(eckit::DumpLoad&);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // Uncomment for persistent, remove otherwise

protected:

    // -- Members
    // None

    // -- Methods

    void print(std::ostream& s) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    // -- Members

    double from_;
    double to_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const StepRange& p) {
        p.print(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

#endif
