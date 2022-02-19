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

	if (!std::regex_match(value, base_match, quantile_regex)) {
		std::ostringstream oss;
		oss << "Quantile " << value << " shall be in the form <integer>:<integer>";
		throw eckit::UserError(oss.str());
	}

    static eckit::Translator<std::string, float> s2l;

	num_ = s2l(base_match.str(1));
	part_ = s2l(base_match.str(2));

	check();
}

void Quantile::check() const {
	if (num_ < 0) {
		std::ostringstream oss;
		oss << "Quantile numerator " << num_ << " shall be non negative";
		throw eckit::UserError(oss.str());
	}
	if (part_ < num_) {
		std::ostringstream oss;
		oss << "Quantile numerator " << num_ << " shall be less or equal the number of partitions " << part_;
		throw eckit::UserError(oss.str());
	}
}

Quantile::Quantile(long num, long part) : num_(num), part_(part) {
	check();
}

Quantile::operator std::string() {
	std::ostringstream oss;
	oss <<  num_ << ':' << part_;
	return oss.str();
}

void Quantile::print(std::ostream& s) const {
	s << num_ << ':' << part_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

