/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/Quantile.h"

#include <stdexcept>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Tokenizer.h"

#include "metkit/mars/TypeInteger.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

Quantile::Quantile(const std::string& value) {
    Tokenizer parse(":");
    std::vector<std::string> result;

    parse(value, result);
    if (result.size() != 2) {
        std::ostringstream oss;
        oss << "Quantile " << value << " must be in the form <integer>[-<integer>]:<integer>";
        throw eckit::BadValue(oss.str());
    }

    try {
        if (result[0].empty() || result[0].front() == '-') {
            std::ostringstream oss;
            oss << "Quantile " << value << " must be in the form <integer>[-<integer>]:<integer>";
            throw eckit::BadValue(oss.str());
        }
        size_t dashPos = result[0].find('-');
        if (dashPos != std::string::npos) {
            min_ = mars::TypeInteger::parse(std::string_view(result[0]).substr(0, dashPos), false);
            max_ = mars::TypeInteger::parse(std::string_view(result[0]).substr(dashPos + 1), false);
        }
        else {
            min_ = max_ = mars::TypeInteger::parse(result[0], false);
        }
        den_ = mars::TypeInteger::parse(result[1], false);
    }
    catch (const std::invalid_argument& e) {
        std::ostringstream oss;
        oss << "Quantile " << value << " must be in the form <integer>[-<integer>]:<integer>";
        throw eckit::BadValue(oss.str());
    }

    check();
}

void Quantile::check() const {
    if (min_ < 0) {
        std::ostringstream oss;
        oss << "Quantile numerator " << min_ << " must be non negative";
        throw eckit::BadValue(oss.str());
    }
    if (max_ < 0) {
        std::ostringstream oss;
        oss << "Quantile numerator " << max_ << " must be non negative";
        throw eckit::BadValue(oss.str());
    }
    if (max_ < min_) {
        std::ostringstream oss;
        oss << "Quantile maximum " << max_ << " must be greater or equal to the minimum " << min_;
        throw eckit::BadValue(oss.str());
    }
    if (den_ < 0) {
        std::ostringstream oss;
        oss << "Quantile denominator " << den_ << " must be non negative";
        throw eckit::BadValue(oss.str());
    }
    if (den_ < max_) {
        std::ostringstream oss;
        oss << "Quantile numerator " << max_ << " must be less or equal the value of denominator " << den_;
        throw eckit::BadValue(oss.str());
    }
}

Quantile::Quantile(long min, long max, long den) : min_(min), max_(max), den_(den) {
    check();
}

Quantile::Quantile(long num, long den) : min_(num), max_(num), den_(den) {
    check();
}

Quantile::operator std::string() {
    std::ostringstream oss;
    if (min_ == max_) {
        oss << min_ << ':' << den_;
    }
    else {
        oss << min_ << '-' << max_ << ':' << den_;
    }
    return oss.str();
}

void Quantile::print(std::ostream& s) const {
    if (min_ == max_) {
        s << min_ << ':' << den_;
    }
    else {
        s << min_ << '-' << max_ << ':' << den_;
    }
}

Quantile& Quantile::operator+=(const long& rhs) {
    min_ += rhs;
    max_ += rhs;
    check();
    return *this;
}
Quantile& Quantile::operator-=(const long& rhs) {
    min_ -= rhs;
    max_ -= rhs;
    check();
    return *this;
}

bool operator==(const Quantile& lhs, const Quantile& rhs) {

    if (lhs.den() != rhs.den()) {
        std::ostringstream oss;
        oss << "Quantile values must belong to the same quantile group";
        throw eckit::BadValue(oss.str());
    }
    return (lhs.min() == rhs.min() && lhs.max() == rhs.max());
}
bool operator<(const Quantile& lhs, const Quantile& rhs) {

    if (lhs.den() != rhs.den()) {
        std::ostringstream oss;
        oss << "Quantile values must belong to the same quantile group";
        throw eckit::BadValue(oss.str());
    }
    return (lhs.min() < rhs.min() || (lhs.min() == rhs.min() && lhs.max() < rhs.max()));
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
