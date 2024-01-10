/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/StepRange.h"
#include "metkit/mars/TypeTime.h"
#include "metkit/mars/TypeRange.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/persist/DumpLoad.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit;


//----------------------------------------------------------------------------------------------------------------------

namespace {

enum TimeUnit {
    Second = 0,
    Minute = 1,
    Hour = 2,
    Day = 3
};

TimeUnit maxUnit(const eckit::Time& t) {
    if (t.seconds() == 0) {
        if (t.minutes() == 0) {
            if (t.hours() == 0) {
                return TimeUnit::Day;
            }
            return TimeUnit::Hour;
        }
        return TimeUnit::Minute;
    }
    return TimeUnit::Second;
}

std::string canonical(const eckit::Time& time) {

    long h = time.hours();
    long m = time.minutes();
    long s = time.seconds();

	// std::cout << h << "h" << m << "m" << s << "s\n";
    std::string out = "";
    if (h!=0 || (m==0 && s==0)) {
        out += std::to_string(h);
        if (m!=0 || s!=0) {
            out += "h";
        }
    }
    if (m!=0) {
        out += std::to_string(m) + "m";
    }
    if (s!=0) {
        out += std::to_string(s) + "s";
    }
    return out;
}

std::string canonical(const eckit::Time& time, TimeUnit unit) {
    switch (unit) {
        case TimeUnit::Second:
            return std::to_string(time.seconds()+60*time.minutes()+3600*time.hours()) + "s";
        case TimeUnit::Minute:
            return std::to_string(time.minutes()+60*time.hours()) + "m";
        case TimeUnit::Day:
        case TimeUnit::Hour:
        default:
            return std::to_string(time.hours());
    }
}

}

namespace metkit::mars {


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
        s << canonical(from_);
    }
    else {
        TimeUnit unit = std::min(maxUnit(from_), maxUnit(to_));            
        s << canonical(from_,unit) << '-' << canonical(to_,unit);
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
			to_ = from_ = eckit::Time(result[0], true);
			break;

		case 2:
			from_ = eckit::Time(result[0], true);
			to_   = eckit::Time(result[1], true);
			break;

		default:
            std::ostringstream msg;
            msg << "Bad StepRange [" << s << "]";
            throw eckit::BadValue(msg.str(), Here());
            break;
    }
}

void StepRange::dump(DumpLoad& a) const
{
	a.dump(from_.seconds()/3600.);
	a.dump(to_.seconds()/3600.);
}

void StepRange::load(DumpLoad& a)
{
	double from, to;
	a.load(from);
	a.load(to);

	from_ = eckit::Time(from*3600., true);
	to_ = eckit::Time(to*3600., true);
}

//----------------------------------------------------------------------------------------------------------------------
} // namespace metkit::mars
