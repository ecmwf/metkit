// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <limits>
#include <ostream>
#include <string>

//-----------------------------------------------------------------------------

namespace eckit {

class Stream;

}

namespace metkit::mars {

//-----------------------------------------------------------------------------

class MarsVerb {
public:  // types

    using value_t = uint8_t;

public:  // methods

    friend std::ostream& operator<<(std::ostream& s, const MarsVerb& x);
    friend eckit::Stream& operator<<(eckit::Stream& s, const MarsVerb& x);
    friend eckit::Stream& operator>>(eckit::Stream& s, MarsVerb& x);

    MarsVerb(value_t v = 0) : value_(v) {}
    explicit MarsVerb(const std::string& s);
    MarsVerb(const MarsVerb& other) : value_(other.value_) {}

public:  // operators

    MarsVerb& operator=(const MarsVerb& other) {
        value_ = other.value_;
        return *this;
    }

    bool operator==(const MarsVerb& other) const { return value_ == other.value_; }

    bool operator<(const MarsVerb& other) const { return value_ < other.value_; }

    operator value_t() const { return value_; }

private:  // members

    value_t value_;

};
//-----------------------------------------------------------------------------

}  // namespace metkit::mars

//-----------------------------------------------------------------------------

template <>
struct std::hash<metkit::mars::MarsVerb> {
    std::size_t operator()(const metkit::mars::MarsVerb& s) const noexcept
    {
        return std::hash<uint8_t>{}(s);
    }
};

template <>
struct std::numeric_limits<metkit::mars::MarsVerb> {
    static metkit::mars::MarsVerb::value_t max() {
        return std::numeric_limits<metkit::mars::MarsVerb::value_t>::max();
    }
};

