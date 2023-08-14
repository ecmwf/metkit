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

#include "metkit/mars/TypeRange.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Tokenizer.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Quantile.h"
#include "metkit/mars/TypesFactory.h"
#include "metkit/mars/TypeTime.h"

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
    long d = time.hours()/24;
    long h = time.hours()%24;
    long m = time.minutes();
    long s = time.seconds();

    std::string out = "";
    if (d!=0) {
        out += std::to_string(d) + "D";
    }
    if (h!=0) {
        out += std::to_string(h) + "h";
    }
    if (m!=0) {
        out += std::to_string(m) + "m";
    }
    if (s!=0) {
        out = std::to_string(s) + "s";
    }
    return out;
}

std::string canonical(const eckit::Time& time, TimeUnit unit) {
    switch (unit) {
        case TimeUnit::Second:
            return std::to_string(time.seconds()) + "s";
        case TimeUnit::Minute:
            return std::to_string(time.minutes()) + "m";
        case TimeUnit::Day:
        case TimeUnit::Hour:
        default:
            return std::to_string(time.hours());
    }
}

}
namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeRange::TypeRange(const std::string &name, const eckit::Value& settings) :
    Type(name, settings),
    by_((std::string)settings["by"]) {

    multiple_ = true;
}


TypeRange::~TypeRange() {
}

void TypeRange::print(std::ostream &out) const {
    out << "TypeRange[name=" << name_ << "]";
}

bool TypeRange::expand(const MarsExpandContext& ctx, std::string& value) const {

	eckit::Tokenizer parse("-");
	std::vector<std::string> result;

	parse(value, result);
    switch (result.size()) {
        case 1: {
            eckit::Time start = eckit::Time(result[0], true);
            value = canonical(start, maxUnit(start));
            return true;
        }
        case 2: {
            eckit::Time start = eckit::Time(result[0], true);
            eckit::Time end = eckit::Time(result[1], true);
            if (start > end) {
                std::ostringstream oss;
                oss << name_ + ": initial value " << start << " cannot be greater that final value " << end;
                throw eckit::BadValue(oss.str());
            }

            TimeUnit unit = std::max(maxUnit(start), maxUnit(end));            
            value = canonical(start, unit) + "-" + canonical(end, unit);
            return true;
        }
        default:
            std::ostringstream oss;
            oss << name_ + ": invalid value " << value << " " << result.size();
            throw eckit::BadValue(oss.str());
    }
    return false;
}

void TypeRange::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

    std::vector<std::string> newval;

    for (size_t i = 0; i < values.size(); ++i) {

        const std::string& s = values[i];

        if (eckit::StringTools::lower(s) == "to" || eckit::StringTools::lower(s) == "t0") {
            std::vector<eckit::Time> tmpval;
            TimeUnit unit;

            if (newval.size() == 0) {
                std::ostringstream oss;
                oss << name_ << " list: 'to' must be preceeded by a starting value.";
                throw eckit::BadValue(oss.str());
            }
            if (values.size() <= i+1) {
                std::ostringstream oss;
                oss << name_ << " list: 'to' must be followed by an ending value.";
                throw eckit::BadValue(oss.str());
            }

            eckit::Time from = eckit::Time(values[i - 1], true);
            tmpval.push_back(from);
            unit = maxUnit(from);            

            eckit::Time to = eckit::Time(values[i + 1], true);
            eckit::Time by = by_;

            if (i+2 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                if (values.size() <= i+3) {
                    std::ostringstream oss;
                    oss << name_ << " list: 'by' must be followed by a step size.";
                    throw eckit::BadValue(oss.str());
                }

                by = eckit::Time(values[i + 3], true);

                i += 2;
            }
//            std::cout << from << " " << to << " " << by << std::endl;

            if (from > to) {
                std::ostringstream oss;
                oss << name_ + ": 'from' value " << from << " cannot be greater that 'to' value " << to;
                throw eckit::BadValue(oss.str());
            }
            if (by <= eckit::Time(0)) {
                std::ostringstream oss;
                oss << name_ + ": 'by' value " << by << " must be a positive number";
                throw eckit::BadValue(name_ + ": 'by' value must be a positive number");
            }
            for (long j = from + by; j <= to; j += by) {
                unit = std::min(unit, maxUnit(j));
            }
            for (long j = from + by; j <= to; j += by) {
                if (unit >= TimeUnit::Hour) {
                    newval.emplace_back(canonical(j, unit));
                } else {
                    newval.emplace_back(canonical(j));
                }
            }

            i++;
        }
        else {
            newval.push_back(tidy(ctx,s));
        }
    }

    std::swap(values, newval);

    Type::expand(ctx, values);
}

static TypeBuilder<TypeRange> type("range");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars
