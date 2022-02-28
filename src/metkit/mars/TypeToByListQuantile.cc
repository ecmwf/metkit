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

#include "metkit/mars/TypeToByListQuantile.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Tokenizer.h"

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
    
    // LOG_DEBUG_LIB(LibMetkit) << "TypeToByListQuantile name=" << name << " settings=" << settings << std::endl;

    eckit::Value values = settings["denominators"];

    if (!values.isList()) {
        values = MarsLanguage::jsonFile(values);
        ASSERT(values.isList());
    }

    for (size_t i = 0; i < values.size(); ++i) {
        const eckit::Value& val = values[i];

        if (val.isNumber()) {
            long v = val;
            if (denominators_.find(v) == denominators_.end()) {
                denominators_.emplace(v);
            }
            else {
                std::ostringstream oss;
                oss << "Redefined " << v << "-quantile";
                throw eckit::SeriousBug(oss.str());
            }
        }
    }

    LOG_DEBUG_LIB(LibMetkit) << "TypeToByListQuantile name=" << name 
                             << " denominators " << denominators_ 
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
    if (denominators_.find(q.den()) == denominators_.end()) {
        std::ostringstream oss;
        oss << name_ << ": " << q.den() << "-quantile not supported.";
        throw eckit::BadValue(oss.str());
    }
    return true;
}

void TypeToByListQuantile::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

    std::vector<std::string> newval;

    for (size_t i = 0; i < values.size(); ++i) {

        const std::string& s = values[i];

        if (eckit::StringTools::lower(s) == "to" || eckit::StringTools::lower(s) == "t0") {
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

            Quantile from = Quantile(tidy(ctx, newval.back()));
            Quantile to = Quantile(tidy(ctx, values[i + 1]));
            long by = by_;

            if (i+2 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                if (values.size() <= i+3) {
                    std::ostringstream oss;
                    oss << name_ << " list: 'by' must be followed by a step size.";
                    throw eckit::BadValue(oss.str());
                }

                eckit::Tokenizer parse(":");
                std::vector<std::string> result;

                parse(values[i + 3], result);
                if (result.size() != 1) {
                    std::ostringstream oss;
                    oss << "by step " << values[i + 3] << " must be a single integer";
                    throw eckit::BadValue(oss.str());
                }

                try {
                    by = std::stol(values[i + 3]);
            	} catch(const std::invalid_argument& e) {
                    std::ostringstream oss;
                    oss << name_ << " list: 'by' must be followed by a single integer number.";
                    throw eckit::BadValue(oss.str());
                }
                i += 2;
            }

            if (from.den() != to.den()) {
                std::ostringstream oss;
                oss << name_ + ": 'from' and 'to' value must belong to the same quantile group";
                throw eckit::BadValue(oss.str());
            }
            if (from.num() > to.num()) {
                std::ostringstream oss;
                oss << name_ + ": 'from' value " << from << " cannot be greater that 'to' value " << to;
                throw eckit::BadValue(oss.str());
            }
            if (by <= 0) {
                std::ostringstream oss;
                oss << name_ + ": 'by' value " << by << " must be a positive number";
                throw eckit::BadValue(name_ + ": 'by' value must be a positive number");
            }
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
