/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <regex>

#include "eckit/utils/Translator.h"
#include "eckit/exception/Exceptions.h"

#include "metkit/mars/Quantile.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

Quantile::Quantile(const std::string &value) {
    const std::regex quantile_regex("([+-]?\\d+):([+-]?\\d+)");
    std::smatch base_match;

    ASSERT_MSG(std::regex_match(value, base_match, quantile_regex), "Quantile " +value+ " shall be in the form <integer>:<integer>");

    static eckit::Translator<std::string, float> s2l;

	num_ = s2l(base_match.str(1));
	den_ = s2l(base_match.str(2));

	ASSERT_MSG(0 <= num_, "Quantile numerator " +std::to_string(num_)+ " shall be non negative");
	ASSERT_MSG(num_ <= den_, "Quantile numerator " +std::to_string(num_)+ " shall less or equal to denominator " +std::to_string(den_));
}

Quantile::Quantile(long num, long den) : num_(num), den_(den) {
	ASSERT_MSG(0 <= num_, "Quantile numerator " +std::to_string(num_)+ " shall be non negative");
	ASSERT_MSG(num_ <= den_, "Quantile numerator " +std::to_string(num_)+ " shall less or equal to denominator " +std::to_string(den_));
}

bool Quantile::operator<(const Quantile& other) {
	return num_<other.num_ && den_ == other.den_;
}

Quantile::operator std::string() {
	std::ostringstream oss;
	oss <<  num_ << ':' << den_;
	return oss.str();
}

void Quantile::print(std::ostream& s) const {
	s << num_ << ':' << den_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

