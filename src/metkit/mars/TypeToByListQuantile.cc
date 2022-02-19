/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#include <regex>

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

    eckit::Value values = settings["partitions"];

    if (!values.isList()) {
        values = MarsLanguage::jsonFile(values);
        ASSERT(values.isList());
    }

    for (size_t i = 0; i < values.size(); ++i) {
        const eckit::Value& val = values[i];

        if (val.isNumber()) {
            long v = val;
            if (partitions_.find(v) == partitions_.end()) {
                partitions_.emplace(v);
            }
            else {
                std::ostringstream oss;
                oss << "Redefined " << v << "-quantile";
                throw eckit::SeriousBug(oss.str());
            }
        }
    }

    LOG_DEBUG_LIB(LibMetkit) << "TypeToByListQuantile name=" << name 
                             << " partitions " << partitions_ 
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
    if (partitions_.find(q.part()) == partitions_.end()) {
        std::ostringstream oss;
        oss << name_ << ": " << q.part() << "-quantile not supported.";
        throw eckit::UserError(oss.str());
    }
    return true;
}

void TypeToByListQuantile::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

    static eckit::Translator<std::string, long> s2l;

    std::vector<std::string> newval;

    for (size_t i = 0; i < values.size(); ++i) {

        const std::string& s = values[i];

        if (eckit::StringTools::lower(s) == "to" || eckit::StringTools::lower(s) == "t0") {
            if (newval.size() == 0) {
                std::ostringstream oss;
                oss << name_ << " list: 'to' must be preceeded by a starting value.";
                throw eckit::UserError(oss.str());
            }
            if (values.size() <= i+1) {
                std::ostringstream oss;
                oss << name_ << " list: 'to' must be followed by an ending value.";
                throw eckit::UserError(oss.str());
            }

            Quantile from = Quantile(tidy(ctx, newval.back()));
            Quantile to = Quantile(tidy(ctx, values[i + 1]));
            long by = by_;

            if (i+2 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                if (values.size() <= i+3) {
                    std::ostringstream oss;
                    oss << name_ << " list: 'by' must be followed by a step size.";
                    throw eckit::UserError(oss.str());
                }
                const std::regex by_regex("(\\d+)");
                std::smatch base_match;

                if (!std::regex_match(values[i + 3], base_match, by_regex)) {
                    std::ostringstream oss;
                    oss << name_ << " list: 'by' must be followed by a single integer number.";
                    throw eckit::UserError(oss.str());
                }
                by = s2l(values[i + 3]);
                i += 2;
            }

            if (from.part() != to.part()) {
                std::ostringstream oss;
                oss << name_ + ": 'from' and 'to' value must belong to the same quantile group";
                throw eckit::UserError(oss.str());
            }
            if (from.num() > to.num()) {
                std::ostringstream oss;
                oss << name_ + ": 'from' value " << from << " cannot be greater that 'to' value " << to;
                throw eckit::UserError(oss.str());
            }
            if (by <= 0) {
                std::ostringstream oss;
                oss << name_ + ": 'by' value " << by << " must be a positive number";
                throw eckit::UserError(name_ + ": 'by' value must be a positive number");
            }
            for (long j = from.num() + by; j <= to.num(); j += by) {
                newval.emplace_back(Quantile(j,from.part()));
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
