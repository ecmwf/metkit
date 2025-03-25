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
#include "metkit/mars/StepRange.h"
#include "metkit/mars/TypeToByList.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class ExtendedTime : public eckit::Time {
    public:
        ExtendedTime(long seconds = 0) : Time(seconds, true) {}
        ExtendedTime(const std::string& time) : Time(time, true) {}
    };

//----------------------------------------------------------------------------------------------------------------------

TypeRange::TypeRange(const std::string &name, const eckit::Value& settings) :
    Type(name, settings) {

    toByList_ = std::make_unique<TypeToByList<StepRange, ExtendedTime>>(this, settings);
    multiple_ = true;
}

TypeRange::~TypeRange() {
}

void TypeRange::print(std::ostream &out) const {
    out << "TypeRange[name=" << name_ << "]";
}

StepRange TypeRange::parse(std::string& value) const {
	eckit::Tokenizer parse("-");
	std::vector<std::string> result;

	parse(value, result);
    switch (result.size()) {
        case 1: {
            return StepRange(eckit::Time(result[0], true));
        }
        case 2: {
            eckit::Time start = eckit::Time(result[0], true);
            eckit::Time end = eckit::Time(result[1], true);
            if (start > end) {
                std::ostringstream oss;
                oss << name_ + ": initial value " << start << " cannot be greater that final value " << end;
                throw eckit::BadValue(oss.str());
            }
            return StepRange(start, end);
        }
        default:
            std::ostringstream oss;
            oss << name_ + ": invalid value " << value << " " << result.size();
            throw eckit::BadValue(oss.str());
    }   
}

bool TypeRange::expand(const MarsExpandContext& ctx, std::string& value) const {

    value = parse(value);
    return true;

}

static TypeBuilder<TypeRange> type("range");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars
