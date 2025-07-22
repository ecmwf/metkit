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
#include "metkit/mars/TypeRange.h"
#include "metkit/mars/TypeTime.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/persist/DumpLoad.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit;


//----------------------------------------------------------------------------------------------------------------------

namespace {

// enum TimeUnit {
//     Second = 0,
//     Minute = 1,
//     Hour = 2,
//     Day = 3
// };

// TimeUnit maxUnit(const eckit::Time& t) {
//     if (t.seconds() == 0) {
//         if (t.minutes() == 0) {
//             if (t.hours() == 0) {
//                 return TimeUnit::Day;
//             }
//             return TimeUnit::Hour;
//         }
//         return TimeUnit::Minute;
//     }
//     return TimeUnit::Second;
// }

std::string canonical(const eckit::Time& time) {

    long h = time.hours();
    long m = time.minutes();
    long s = time.seconds();

    std::string out = "";
    if (h != 0 || (m == 0 && s == 0)) {
        out += std::to_string(h);
        if (m != 0 || s != 0) {
            out += "h";
        }
    }
    if (m != 0) {
        out += std::to_string(m) + "m";
    }
    if (s != 0) {
        out += std::to_string(s) + "s";
    }
    return out;
}

// std::string canonical(const eckit::Time& time, TimeUnit unit) {
//     switch (unit) {
//         case TimeUnit::Second:
//             return std::to_string(time.seconds()+60*time.minutes()+3600*time.hours()) + "s";
//         case TimeUnit::Minute:
//             return std::to_string(time.minutes()+60*time.hours()) + "m";
//         case TimeUnit::Day:
//         case TimeUnit::Hour:
//         default:
//             return std::to_string(time.hours());
//     }
// }

}  // namespace

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

StepRange::operator std::string() const {
    std::ostringstream os;
    os << *this;
    return os.str();
}

void StepRange::print(std::ostream& s) const {
    if (from_ == to_) {
        s << canonical(eckit::Time(std::lround(from_ * 3600.), true));
    }
    else {
        eckit::Time f{std::lround(from_ * 3600.), true};
        eckit::Time t{std::lround(to_   * 3600.), true};

        s << canonical(f) << '-' << canonical(t);
    }
}

StepRange::StepRange(const std::string& s) : from_(0.), to_(0.) {
    Tokenizer parse("-");
    std::vector<std::string> result;

    parse(s, result);

    switch (result.size()) {
        case 1:
            to_ = from_ = eckit::Time(result[0], true) / 3600.;
            break;

        case 2:
            from_ = eckit::Time(result[0], true) / 3600.;
            to_   = eckit::Time(result[1], true) / 3600.;
            break;

        default:
            std::ostringstream msg;
            msg << "Bad StepRange [" << s << "]";
            throw eckit::BadValue(msg.str(), Here());
            break;
    }
}

void StepRange::dump(DumpLoad& a) const {
    a.dump(from_);
    a.dump(to_);
}

void StepRange::load(DumpLoad& a) {
    a.load(from_);
    a.load(to_);
}

//----------------------------------------------------------------------------------------------------------------------
}  // namespace metkit::mars
