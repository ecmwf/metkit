/*
 * (C) Copyright 1996- ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/StepRange.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/persist/DumpLoad.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

StepRange::operator std::string() const
{
    std::ostringstream os;
    os << *this;
    return os.str();
}

void StepRange::print(std::ostream& s) const
{
	if(from_ == to_) {
        s << from_;
    }
    else {
        s << from_ << '-';
        s << to_;
    }
}

StepRange::StepRange(const std::string& s):
	from_(0),
	to_(0)
{
	Tokenizer parse("-");
	std::vector<std::string> result;

	parse(s,result);

	switch(result.size())
	{
		case 1:
			from_ = to_ = atof(result[0].c_str());
			break;

		case 2:
			from_ = atof(result[0].c_str());
			to_   = atof(result[1].c_str());
			break;

		default:
			ASSERT(1 == 0);
			break;
	}
	
}

void StepRange::dump(DumpLoad& a) const
{
	a.dump(from_);
	a.dump(to_);
}

void StepRange::load(DumpLoad& a)
{
	a.load(from_);
	a.load(to_);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
