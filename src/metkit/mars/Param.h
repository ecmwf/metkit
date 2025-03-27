/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File Param.h
// Baudouin Raoult - ECMWF Dec 97

#ifndef metkit_Param_H
#define metkit_Param_H

#include <string>

#include "eckit/persist/Bless.h"

namespace eckit {
class DumpLoad;
}

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class Param {
public:

    Param() : table_(-1), value_(-1) {}

    Param(const std::string&);

    Param(long table, long value) : table_(table), value_(value) {}

#include "metkit/mars/Param.b"

    ~Param() {}

    operator std::string() const;

    bool operator==(const Param& other) const { return value_ == other.value_ && table_ == other.table_; }

    bool operator!=(const Param& other) const { return value_ != other.value_ || table_ != other.table_; }

    bool operator<(const Param& other) const {
        return (value_ == other.value_) ? (table_ < other.table_) : (value_ < other.value_);
    }

    bool operator>(const Param& other) const {
        return (value_ == other.value_) ? (table_ > other.table_) : (value_ > other.value_);
    }

    long table() const { return table_; }
    long value() const { return value_; }
    long grib1value() const { return value_ % 1000; }

    long paramId() const;

    void dump(eckit::DumpLoad&) const;
    void load(eckit::DumpLoad&);

protected:

    void print(std::ostream& s) const;

private:

    long table_;
    long value_;

    friend std::ostream& operator<<(std::ostream& s, const Param& p) {
        p.print(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit

#endif
