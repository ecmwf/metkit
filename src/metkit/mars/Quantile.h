/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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

    long num() const { return num_; }
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

    long num_;
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
