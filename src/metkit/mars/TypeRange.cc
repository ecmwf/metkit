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
#include "metkit/mars/StepRange.h"
#include "metkit/mars/TypeTime.h"
#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class ExtendedTime : public eckit::Time {
public:

    ExtendedTime(long seconds = 0) : Time(seconds, true) {}
    ExtendedTime(const std::string& time) : Time(time, true) {}
};

//----------------------------------------------------------------------------------------------------------------------

TypeRange::TypeRange(const std::string& name, const eckit::Value& settings) : Type(name, settings) {

    toByList_ = std::make_unique<TypeToByList<StepRange, ExtendedTime>>(*this, settings);
    multiple_ = true;
}

void TypeRange::print(std::ostream& out) const {
    out << "TypeRange[name=" << name_ << "]";
}

StepRange TypeRange::parse(const std::string& value) const {
    return StepRange{value};
}

bool TypeRange::expand(std::string& value, const MarsRequest&) const {

    value = parse(value);
    return true;
}

static TypeBuilder<TypeRange> type("range");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
