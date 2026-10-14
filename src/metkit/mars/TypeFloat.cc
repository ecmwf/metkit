// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "eckit/utils/Translator.h"

#include "metkit/mars/MarsRequest.h"

#include "metkit/mars/TypeFloat.h"
#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeFloat::TypeFloat(const std::string& type, Keyword keyword, const eckit::Value& settings) :
    Type(type, keyword, settings) {}

TypeFloat::TypeFloat(const std::string& type, Keyword key, MemFile& file) : Type(type, key, file) {}

bool TypeFloat::expand(std::string& value, const MarsRequest&) const {

    bool dot = false;

    for (std::string::const_iterator j = value.begin(); j != value.end(); ++j) {
        switch (*j) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
                break;
            case '.':
                if (dot) {
                    return false;
                }
                dot = true;
                break;
            default:
                return false;
        }
    }

    // Strip leading zeros
    while (value.size() && value[0] == '0') {
        value = value.substr(1);
    }

    // Strip trailing zeros
    if (dot) {
        while (value.size() && value[value.size() - 1] == '0') {
            value = value.substr(0, value.size() - 1);
        }
        if (value.size() && value[value.size() - 1] == '.') {
            value = value.substr(0, value.size() - 1);
        }
    }

    if (value.empty()) {
        value = "0";
    }
    return true;
}

void TypeFloat::print(std::ostream& out) const {
    out << "TypeFloat[name=" << name() << "]";
}

static TypeBuilder<TypeFloat> type("float");

//----------------------------------------------------------------------------------------------------------------------


class TypeToByListFloat : public TypeFloat {
public:

    TypeToByListFloat(const std::string& type, Keyword key, const eckit::Value& val) : TypeFloat(type, key, val) {
        toByList_ = std::make_unique<TypeToByList<float, float>>(*this, val);
        flags_[1] = true;
    }
    TypeToByListFloat(const std::string& type, Keyword key, MemFile& file) : TypeFloat(type, key, file) {
        toByList_ = std::make_unique<TypeToByList<float, float>>(*this, file);
        flags_[1] = true;
    }

    ~TypeToByListFloat() noexcept override = default;


protected:

    void print(std::ostream& out) const override { out << "TypeToByListFloat[name=" << name() << "]"; }
};

static TypeBuilder<TypeToByListFloat> typeList("to-by-list-float");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
