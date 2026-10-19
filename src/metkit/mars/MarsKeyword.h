// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <ostream>

//-----------------------------------------------------------------------------

namespace eckit {

class Stream;

}

//-----------------------------------------------------------------------------

namespace metkit::mars {

//-----------------------------------------------------------------------------

class MarsKeyword {
public:  // types

    using value_t = uint16_t;

public:  // methods

    friend std::ostream& operator<<(std::ostream& s, const MarsKeyword& x);
    friend eckit::Stream& operator<<(eckit::Stream& s, const MarsKeyword& x);
    friend eckit::Stream& operator>>(eckit::Stream& s, MarsKeyword& x);

    // -- Contructors

    MarsKeyword(value_t v = 0) : value_(v) {}
    explicit MarsKeyword(const std::string& s);
    MarsKeyword(const MarsKeyword& other) : value_(other.value_) {}

    static MarsKeyword unknown() { return MarsKeyword{}; }
public:  // operators

    MarsKeyword& operator=(const MarsKeyword& other) {
        value_ = other.value_;
        return *this;
    }

    bool operator==(const MarsKeyword& other) const { return value_ == other.value_; }
    bool operator<(const MarsKeyword& other) const { return value_ < other.value_; }

    operator value_t() const { return value_; }

private:  // members

    value_t value_;

};
    
//-----------------------------------------------------------------------------

}  // namespace metkit::mars

//-----------------------------------------------------------------------------

template <>
struct std::hash<metkit::mars::MarsKeyword> {
    std::size_t operator()(const metkit::mars::MarsKeyword& s) const noexcept
    {
        return std::hash<uint16_t>{}(s);
    }
};

template <>
struct std::numeric_limits<metkit::mars::MarsKeyword> {
    static metkit::mars::MarsKeyword::value_t max() {
        return std::numeric_limits<metkit::mars::MarsKeyword::value_t>::max();
    }
};
