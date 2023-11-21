/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "TypeToByListFloat.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/TypesFactory.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeToByListFloat::TypeToByListFloat(const std::string &name, const eckit::Value& settings) :
    TypeFloat(name, settings),
    by_(settings["by"]) {

    multiple_ = true;

}

TypeToByListFloat::~TypeToByListFloat() {
}

void TypeToByListFloat::print(std::ostream &out) const {
    out << "TypeToByListFloat[name=" << name_ << "]";
}


void TypeToByListFloat::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

    static eckit::Translator<std::string, float> s2l;
    static eckit::Translator<float, std::string> l2s;

    std::vector<std::string> newval;

    for (size_t i = 0; i < values.size(); ++i) {

        const std::string& s = values[i];

        if (eckit::StringTools::lower(s) == "to" || eckit::StringTools::lower(s) == "t0") {
            ASSERT(newval.size() > 0);
            ASSERT(i + 1 < values.size());

            float from = s2l(tidy(ctx, newval.back()));
            float to = s2l(tidy(ctx, values[i + 1]));
            float by = by_;

            if (i + 3 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                by = s2l(tidy(ctx, values[i + 3]));
                i += 2;
            }

            if (by > 0) {
                ASSERT_MSG(from <= to, name_ + ": [" + std::to_string(from) + "] value must be less than [" +
                                           std::to_string(to) + "] value!");
                for (float j = from + by; j <= to; j += by) { newval.push_back(l2s(j)); }
            } else if (by < 0) {
                ASSERT_MSG(from >= to, name_ + ": [" + std::to_string(from) + "] value must be greater than [" +
                                           std::to_string(to) + "] value!");
                for (float j = from + by; j >= to; j += by) { newval.push_back(l2s(j)); }
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

static TypeBuilder<TypeToByListFloat> type("to-by-list-float");

//----------------------------------------------------------------------------------------------------------------------

} // namespace mars
} // namespace metkit
