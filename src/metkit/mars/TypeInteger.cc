/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/TypeInteger.h"

#include "eckit/utils/Translator.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeInteger::TypeInteger(const std::string& type, Keyword key, const eckit::Value& settings) :
    Type(type, key, settings) {
    // check if the settings contain a range
    if (settings.contains("range") && settings["range"].size() == 2) {
        range_ = {settings["range"][0], settings["range"][1]};
    }
}

TypeInteger::TypeInteger(const std::string& type, Keyword key, MemFile& file) : Type(type, key, file) {
    // check if the file contains a range
    uint8_t hasRange = file.read8();
    if (hasRange) {
        range_         = Range{};
        uint32_t lower = file.read32();
        uint32_t upper = file.read32();
        memcpy(&range_->lower_, &lower, sizeof(lower));
        memcpy(&range_->upper_, &upper, sizeof(upper));
    }
}

void TypeInteger::write(std::ofstream& file) const {
    // TypeInteger reads its range before TypeToByListInt reads the by-value: keep the same order
    writeCommon(file);

    if (range_) {
        write8(file, uint8_t(1));
        uint32_t lower, upper;
        memcpy(&lower, &range_->lower_, sizeof(lower));
        memcpy(&upper, &range_->upper_, sizeof(upper));
        write32(file, lower);
        write32(file, upper);
    }
    else {
        write8(file, uint8_t(0));
    }

    writeToByList(file);
}

void TypeInteger::print(std::ostream& out) const {
    out << "TypeInteger[name=" << name() << "]";
}

bool TypeInteger::ok(const std::string& value, long& n) const {
    n         = 0;
    long sign = 1;
    for (std::string::const_iterator j = value.begin(); j != value.end(); ++j) {
        switch (*j) {
            case '-':
                if (j == value.begin()) {
                    sign = -1;
                }
                else {
                    return false;
                }
                break;

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
                n *= 10;
                n += (*j) - '0';
                break;

            default:
                return false;
        }
    }
    n *= sign;

    return !range_ || (n >= range_->lower_ && n <= range_->upper_);
}

bool TypeInteger::expand(std::string& value, const MarsRequest&) const {
    long n = 0;
    if (ok(value, n)) {
        static eckit::Translator<long, std::string> l2s;
        value = l2s(n);
        return true;
    }
    return false;
}

static TypeBuilder<TypeInteger> type("integer");

//----------------------------------------------------------------------------------------------------------------------

class TypeToByListInt : public TypeInteger {

public:

    TypeToByListInt(const std::string& type, Keyword key, const eckit::Value& settings) :
        TypeInteger(type, key, settings) {
        toByList_ = std::make_unique<TypeToByList<long, long>>(*this, settings);
        flags_[1] = true;
    }
    TypeToByListInt(const std::string& name, Keyword key, MemFile& file) : TypeInteger(name, key, file) {
        toByList_ = std::make_unique<TypeToByList<long, long>>(*this, file);
        flags_[1] = true;
    }

protected:

    void print(std::ostream& out) const override { out << "TypeToByListInt[name=" << name() << "]"; }
};

static TypeBuilder<TypeToByListInt> typeList("to-by-list");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
