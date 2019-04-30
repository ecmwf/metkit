/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/Param.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/persist/DumpLoad.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------


Param::Param(const std::string& s)
{
	Tokenizer parse(".");
	std::vector<std::string> result;

	parse(s, result);
	switch (result.size())
	{
	case 1:
		value_ = atol(result[0].c_str());
		table_ = 0;
		break;

	case 2:
		value_ = atol(result[0].c_str());
		table_ = atol(result[1].c_str());
		break;

	default:
		throw eckit::UserError("Invalid param");
	}

}

Param::operator std::string() const
{
	std::ostringstream os;
	os << *this;
	return os.str();
}

void Param::print(std::ostream& s) const
{
	if (table_)
		s << value_ << '.' << table_;
	else
		s << value_ ;
}

void Param::dump(DumpLoad& a) const
{
	a.dump(value_);
	a.dump(table_);
}

void Param::load(DumpLoad& a)
{
	a.load(value_);
	a.load(table_);
}

long Param::paramId() const {
	if (table_ == 128) {
		return value_;
	}
	return table_ * 1000 + value_;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

