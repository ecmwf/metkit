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

#include "eckit/types/Date.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars/TypesFactory.h"
#include "metkit/mars/TypeDate.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsExpandContext.h"


namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeDate::TypeDate(const std::string &name, const eckit::Value& settings) :
    Type(name, settings), TypeToByList<eckit::Date, long>(name, settings) {

    DummyContext ctx;
    multiple_ = true;
}

TypeDate::~TypeDate() {
}

void TypeDate::pass2(const MarsExpandContext& ctx, MarsRequest& request) {
    std::vector<std::string> values = request.values(name_, true);
    Type::expand(ctx, values);
    request.setValuesTyped(this, values);
}

bool TypeDate::expand(const MarsExpandContext& ctx, std::string &value) const {
    if (!value.empty()) {
        eckit::Translator<long, std::string> l2s;
        if (value[0] == '0' || value[0] == '-') {
            eckit::Translator<std::string, long> s2l;
            long n = s2l(value);
            if (n <= 0) {
                eckit::Date now(n);
                value = l2s(now.yyyymmdd());
            }
        }
        else {
            eckit::Date date(value);
            value = l2s(date.yyyymmdd());
        }
    }
    return true;
}

void TypeDate::print(std::ostream & out) const {
    out << "TypeDate[name=" << name_ << "]";
}

static TypeBuilder<TypeDate> type("date");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars
