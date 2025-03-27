/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <stdexcept>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Tokenizer.h"

#include "metkit/mars/Quantile.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

Quantile::Quantile(const std::string& value) {
    Tokenizer parse(":");
    std::vector<std::string> result;

    parse(value, result);
    if (result.size() != 2) {
        std::ostringstream oss;
        oss << "Quantile " << value << " must be in the form <integer>:<integer>";
        throw eckit::BadValue(oss.str());
    }

    try {
        num_ = std::stol(result[0]);
        den_ = std::stol(result[1]);
    }
    catch (const std::invalid_argument& e) {
        std::ostringstream oss;
        oss << "Quantile " << value << " must be in the form <integer>:<integer>";
        throw eckit::BadValue(oss.str());
    }

    check();
}

void Quantile::check() const {
    if (num_ < 0) {
        std::ostringstream oss;
        oss << "Quantile numerator " << num_ << " must be non negative";
        throw eckit::BadValue(oss.str());
    }
    if (den_ < 0) {
        std::ostringstream oss;
        oss << "Quantile denominator " << den_ << " must be non negative";
        throw eckit::BadValue(oss.str());
    }
    if (den_ < num_) {
        std::ostringstream oss;
        oss << "Quantile numerator " << num_ << " must be less or equal the value of denominator " << den_;
        throw eckit::BadValue(oss.str());
    }
}

Quantile::Quantile(long num, long den) : num_(num), den_(den) {
    check();
}

Quantile::operator std::string() {
    std::ostringstream oss;
    oss << num_ << ':' << den_;
    return oss.str();
}

void Quantile::print(std::ostream& s) const {
    s << num_ << ':' << den_;
}

Quantile& Quantile::operator+=(const long& rhs) {
    num_ += rhs;
    check();
    return *this;
}

bool operator==(const Quantile& lhs, const Quantile& rhs) {

    if (lhs.den() != rhs.den()) {
        std::ostringstream oss;
        oss << "Quantile values must belong to the same quantile group";
        throw eckit::BadValue(oss.str());
    }
    return (lhs.num() == rhs.num());
}
bool operator<(const Quantile& lhs, const Quantile& rhs) {

    if (lhs.den() != rhs.den()) {
        std::ostringstream oss;
        oss << "Quantile values must belong to the same quantile group";
        throw eckit::BadValue(oss.str());
    }
    return (lhs.num() < rhs.num());
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
