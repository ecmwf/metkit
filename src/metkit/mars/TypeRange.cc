// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0
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

TypeRange::TypeRange(const std::string& type, MarsKeyword key, const eckit::Value& vals) : Type(type, key, vals) {
    toByList_ = std::make_unique<TypeToByList<StepRange, ExtendedTime>>(*this, vals);
    flags_[1] = true;
}
TypeRange::TypeRange(const std::string& type, MarsKeyword key, MMapFileReader& file) : Type(type, key, file) {
    toByList_ = std::make_unique<TypeToByList<StepRange, ExtendedTime>>(*this, file);
    flags_[1] = true;
}

void TypeRange::print(std::ostream& out) const {
    out << "TypeRange[name=" << name() << "]";
}

bool TypeRange::expand(std::string& value, const MarsRequest&) const {

    value = StepRange{value};
    return true;
}

static TypeBuilder<TypeRange> type("range");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
