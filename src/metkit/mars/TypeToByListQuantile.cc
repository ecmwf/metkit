/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/TypeToByListQuantile.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/utils/Translator.h"
#include "eckit/utils/StringTools.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Quantile.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeToByListQuantile::TypeToByListQuantile(const std::string &name, const eckit::Value& settings) :
    Type(name, settings),
    by_(settings["by"]) {
    
    LOG_DEBUG_LIB(LibMetkit) << "TypeToByListQuantile name=" << name << " settings=" << settings << std::endl;

    eckit::Value values = settings["values"];

    if (!values.isList()) {
        values = MarsLanguage::jsonFile(values);
        ASSERT(values.isList());
    }

    for (size_t i = 0; i < values.size(); ++i) {
        const eckit::Value& val = values[i];

        // LOG_DEBUG_LIB(LibMetkit) << "val : " << val << std::endl;

        if (val.isNumber()) {
            long v = val;
            if (values_.find(v) == values_.end()) {
                values_.emplace(v);
            }
            else {
                std::ostringstream oss;
                oss << "Redefined quantile '" << v << "'";
                throw eckit::SeriousBug(oss.str());
            }
        }
    }

    LOG_DEBUG_LIB(LibMetkit) << "TypeToByListQuantile name=" << name 
                             << " values " << values_ 
                             << std::endl;

    multiple_ = true;
}

TypeToByListQuantile::~TypeToByListQuantile() {
}

void TypeToByListQuantile::print(std::ostream &out) const {
    out << "TypeToByListQuantile[name=" << name_ << "]";
}

bool TypeToByListQuantile::expand(const MarsExpandContext& ctx, std::string& value) const {
    Quantile q(value);
    ASSERT_MSG(values_.find(q.den()) != values_.end(), name_ + ": quantile denominator " +std::to_string(q.den())+ " not supported.");
    return true;
}

void TypeToByListQuantile::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

    static eckit::Translator<std::string, float> s2l;

    std::vector<std::string> newval;

    for (size_t i = 0; i < values.size(); ++i) {

        const std::string& s = values[i];

        if (eckit::StringTools::lower(s) == "to" || eckit::StringTools::lower(s) == "t0") {
            ASSERT(newval.size() > 0);
            ASSERT(i + 1 < values.size());

            Quantile from = Quantile(tidy(ctx, newval.back()));
            Quantile to = Quantile(tidy(ctx, values[i + 1]));
            long by = by_;

            if (i + 3 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                by = s2l(values[i + 3]);
                i += 2;
            }

            ASSERT_MSG(from.num() <= to.num(), name_ + ": 'from' value must be less that 'to' value");
            ASSERT_MSG(from.den() == to.den(), name_ + ": 'from' and 'to' value must belong to the same quantile group");
            ASSERT_MSG(by > 0, name_ + ": 'by' value must be a positive number");
            for (long j = from.num() + by; j <= to.num(); j += by) {
                newval.emplace_back(Quantile(j,from.den()));
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

static TypeBuilder<TypeToByListQuantile> type("to-by-list-quantile");

//----------------------------------------------------------------------------------------------------------------------

} // namespace mars
} // namespace metkit
