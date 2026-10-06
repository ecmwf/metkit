/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars/TypeRegex.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/utils/StringTools.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeRegex::TypeRegex(const std::string& type, Keyword key, const eckit::Value& vals) : Type(type, key, vals) {

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
TypeRegex::TypeRegex(const std::string& type, Keyword key, MemFile& file) : Type(type, key, file) {
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

}  // namespace mars
}  // namespace metkit
