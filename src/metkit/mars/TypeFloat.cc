/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/utils/Translator.h"

#include "metkit/mars/MarsRequest.h"

#include "metkit/mars/TypeFloat.h"
#include "metkit/mars/TypesFactory.h"
#include "metkit/mars/TypeFloat.h"
#include "metkit/mars/TypeToByList.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeFloat::TypeFloat(const std::string& name, const eckit::Value& settings) : Type(name, settings) {}

TypeFloat::~TypeFloat() {}

bool TypeFloat::expand(const MarsExpandContext& ctx, std::string& value) const {

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
                dot = true;
                break;


            default:
                return false;
                // throw eckit::UserError(name_ + ": invalid float '" + value + "'");
                break;
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

    // static eckit::Translator<std::string, double> s2d;
    // static eckit::Translator<double, std::string> d2s;
    // value = d2s(s2d(value));
    return true;
}

void TypeFloat::print(std::ostream &out) const {
    out << "TypeFloat[name=" << name() << "]";
}

static TypeBuilder<TypeFloat> type("float");

//----------------------------------------------------------------------------------------------------------------------

TypeToByListFloat::TypeToByListFloat(const std::string &name, const eckit::Value& settings) :
    TypeFloat(name, settings) {

    toByList_ = std::make_unique<TypeToByList<float, float>>(this, settings);
    multiple_ = true;
}

void TypeToByListFloat::print(std::ostream &out) const {
    out << "TypeToByListFloat[name=" << name()  << "]";
}

static TypeBuilder<TypeToByListFloat> typeList("to-by-list-float");

//----------------------------------------------------------------------------------------------------------------------

TypeToByListFloat::TypeToByListFloat(const std::string &name, const eckit::Value& settings) :
    TypeFloat(name, settings) {

    toByList_ = std::make_unique<TypeToByList<float, float>>(this, settings);
    multiple_ = true;
}

void TypeToByListFloat::print(std::ostream &out) const {
    out << "TypeToByListFloat[name=" << name()  << "]";
}

static TypeBuilder<TypeToByListFloat> typeList("to-by-list-float");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
