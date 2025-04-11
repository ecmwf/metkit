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
#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeToByListQuantile::TypeToByListQuantile(const std::string& name, const eckit::Value& settings) :
    Type(name, settings) {

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

    LOG_DEBUG_LIB(LibMetkit) << "TypeToByListQuantile name=" << name << " denominators " << denominators_ << std::endl;

    toByList_ = std::make_unique<TypeToByList<Quantile, long>>(*this, settings);
    multiple_ = true;
}

void TypeToByListQuantile::print(std::ostream& out) const {
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

static TypeBuilder<TypeToByListQuantile> type("to-by-list-quantile");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
