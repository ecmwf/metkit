// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   Quantile.h
/// @author Emanuele Danovaro
/// @date   February 2022

#pragma once

#include <string>

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class Quantile {
public:

    Quantile(const std::string& value);
    Quantile(long num, long den);
    Quantile(long min, long max, long den);

    long num() const { return max_; }
    long min() const { return min_; }
    long max() const { return max_; }
    long den() const { return den_; }

    operator std::string();

    Quantile& operator+=(const long& rhs);
    Quantile& operator-=(const long& rhs);

protected:

    void print(std::ostream& s) const;

private:

    void check() const;

    friend std::ostream& operator<<(std::ostream& s, const Quantile& q) {
        q.print(s);
        return s;
    }

private:

    long min_;
    long max_;
    long den_;
};

bool operator==(const Quantile& lhs, const Quantile& rhs);
inline bool operator!=(const Quantile& lhs, const Quantile& rhs) {
    return !(rhs == lhs);
}
bool operator<(const Quantile& lhs, const Quantile& rhs);
inline bool operator>(const Quantile& lhs, const Quantile& rhs) {
    return rhs < lhs;
}
inline bool operator<=(const Quantile& lhs, const Quantile& rhs) {
    return !(lhs > rhs);
}
inline bool operator>=(const Quantile& lhs, const Quantile& rhs) {
    return !(lhs < rhs);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
