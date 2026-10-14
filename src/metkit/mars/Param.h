// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File Param.h
// Baudouin Raoult - ECMWF Dec 97

#pragma once

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
