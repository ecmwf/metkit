// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/mars/TypeRegex.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/utils/StringTools.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeRegex::TypeRegex(const std::string& name, const eckit::Value& settings) : Type(name, settings), uppercase_(false) {

    if (settings.contains("uppercase")) {
        uppercase_ = settings["uppercase"];
    }

    eckit::Value r = settings["regex"];

    if (r.isList()) {
        for (size_t i = 0; i < r.size(); ++i) {
            regex_.push_back(std::string(r[i]));
        }
    }
    else {
        regex_.push_back(std::string(r));
    }
}

bool TypeRegex::expand(std::string& value, const MarsRequest&) const {

    for (std::vector<eckit::Regex>::const_iterator j = regex_.begin(); j != regex_.end(); ++j) {
        if ((*j).match(value)) {
            if (uppercase_) {
                value = eckit::StringTools::upper(value);
            }
            return true;
        }
    }

    return false;
}


void TypeRegex::print(std::ostream& out) const {
    out << "TypeRegex[name=" << name_ << "]";
}


static TypeBuilder<TypeRegex> type("regex");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
