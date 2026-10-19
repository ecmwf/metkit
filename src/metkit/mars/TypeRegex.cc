// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/mars/TypeRegex.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/utils/StringTools.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeRegex::TypeRegex(const std::string& type, MarsKeyword key, const eckit::Value& vals) : Type(type, key, vals) {

    eckit::Value r = vals["regex"];

    if (r.isList()) {
        for (size_t i = 0; i < r.size(); ++i) {
            regex_.push_back(std::string(r[i]));
        }
    }
    else {
        regex_.push_back(std::string(r));
    }
}
TypeRegex::TypeRegex(const std::string& type, MarsKeyword key, MemFile& file) : Type(type, key, file) {
    uint8_t numRegex = file.read8();
    for (uint8_t i = 0; i < numRegex; ++i) {
        regex_.emplace_back(std::string{file.readString()});
    }
}

void TypeRegex::write(std::ofstream& file) const {
    Type::write(file);
    write8(file, regex_.size());
    for (const auto& r : regex_) {
        // the pattern itself, not operator<< which wraps it in slashes
        writeString(file, static_cast<const std::string&>(r));
    }
}


bool TypeRegex::expand(std::string& value, const MarsRequest&) const {

    for (std::vector<eckit::Regex>::const_iterator j = regex_.begin(); j != regex_.end(); ++j) {
        if ((*j).match(value)) {
            if (flags_[3]) {
                value = eckit::StringTools::upper(value);
            }
            return true;
        }
    }

    return false;
}


void TypeRegex::print(std::ostream& out) const {
    out << "TypeRegex[name=" << name() << "]";
}


static TypeBuilder<TypeRegex> type("regex");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
