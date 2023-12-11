/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <iomanip>
#include <regex>

#include "eckit/utils/Translator.h"
#include "eckit/types/Date.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypesFactory.h"
#include "metkit/mars/TypeTime.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeTime::TypeTime(const std::string &name, const eckit::Value& settings) :
    Type(name, settings), by_(6*3600) {
}

TypeTime::~TypeTime() {
}

bool TypeTime::expand(const MarsExpandContext&, std::string &value) const {

    eckit::Time time(value);
    
    std::ostringstream oss;
    if (time.seconds() != 0) {
        oss << "Cannot normalise time '" << value << "' - seconds not supported";
        throw eckit::SeriousBug(oss.str(), Here());
    }
    if (time.hours() >= 24) {
        oss << "Cannot normalise time '" << value << "' - " << time.hours() << " hours > 24 not supported";
        throw eckit::SeriousBug(oss.str(), Here());
    }

    oss << std::setfill('0') << std::setw(2) << time.hours() << std::setfill('0') << std::setw(2) << time.minutes();
    value = oss.str();
    
    return true;
}


void TypeTime::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

    //static eckit::Translator<std::string, long> s2l;
    //static eckit::Translator<long, std::string> l2s;

    std::vector<std::string> newval;

    for (size_t i = 0; i < values.size(); ++i) {

        const std::string& s = values[i];

        if (eckit::StringTools::lower(s) == "to") {
            ASSERT(newval.size() > 0);
            ASSERT(i + 1 < values.size());

            long from = ((eckit::Second) eckit::Time(tidy(ctx, newval.back())));
            long to   = ((eckit::Second) eckit::Time(tidy(ctx, values[i + 1])));
            ASSERT(from <= to);
            long by = by_;

            if (i + 3 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                by = ((eckit::Second) eckit::Time(tidy(ctx, values[i + 3])));
                i += 2;
            }

            for (long j = from + by; j <= to; j += by) {
                newval.push_back(eckit::Time(j));
            }

            i++;

        }
        else {
            newval.push_back(s);
        }
    }

    std::swap(values, newval);
    Type::expand(ctx, values);
}

void TypeTime::print(std::ostream &out) const {
    out << "TypeTime[name=" << name_ << "]";
}

static TypeBuilder<TypeTime> type("time");

//----------------------------------------------------------------------------------------------------------------------

} // namespace mars
} // namespace metkit
