// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <iomanip>
#include <regex>

#include "metkit/mars/TypeTime.h"

#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeTime::TypeTime(const std::string& name, const eckit::Value& settings) : Type(name, settings) {

    toByList_ = std::make_unique<TypeToByList<eckit::Time, eckit::Time>>(*this, settings);
    multiple_ = true;
}

bool TypeTime::expand(std::string& value, const MarsRequest&) const {

    eckit::Time time(value);

    std::ostringstream oss;
    if (time.seconds() != 0) {
        oss << "Cannot normalise time '" << value << "' - seconds not supported";
        throw eckit::BadValue(oss.str(), Here());
    }
    if (time.hours() >= 24) {
        oss << "Cannot normalise time '" << value << "' - " << time.hours() << " hours > 24 not supported";
        throw eckit::BadValue(oss.str(), Here());
    }

    oss << std::setfill('0') << std::setw(2) << time.hours() << std::setfill('0') << std::setw(2) << time.minutes();
    value = oss.str();
    return true;
}

void TypeTime::print(std::ostream& out) const {
    out << "TypeTime[name=" << name_ << "]";
}

static TypeBuilder<TypeTime> type("time");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
