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
#include "metkit/MarsRequest.h"

#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeDate.h"
#include "eckit/utils/StringTools.h"

#include "metkit/MarsExpandContext.h"


namespace metkit {

class DummyContext: public MarsExpandContext {
    virtual void info(std::ostream&) const {}
};

//----------------------------------------------------------------------------------------------------------------------

TypeDate::TypeDate(const std::string &name, const eckit::Value& settings) :
    Type(name, settings),
    by_(1) {

    DummyContext ctx;

    for (size_t i = 0; i < originalDefaults_.size(); i++ ) {
        originalDefaults_[i] = tidy(ctx, originalDefaults_[i]);
    }

    defaults_ = originalDefaults_;
}

TypeDate::~TypeDate() {
}

bool TypeDate::expand(const MarsExpandContext& ctx, std::string &value) const {
    if (!value.empty() && (value[0] == '0' || value[0] == '-')) {
        eckit::Translator<std::string, long> t;
        long n = t(value);
        if (n <= 0) {
            eckit::Date now(n);
            eckit::Translator<long, std::string> t;
            value = t(now.yyyymmdd());
        }
    }
    return true;

}


void TypeDate::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

    static eckit::Translator<std::string, long> s2l;
    static eckit::Translator<long, std::string> l2s;

    if (values.size() == 3) {
        if (eckit::StringTools::lower(values[1])[0] == 't') {
            eckit::Date from = tidy(ctx, values[0]);
            eckit::Date to = tidy(ctx, values[2]);
            long by = by_;
            values.clear();
            values.reserve((to - from) / by + 1);
            for (eckit::Date i = from; i <= to; i += by) {
                values.push_back(l2s(i.yyyymmdd()));
            }
            return;
        }
    }

    if (values.size() == 5) {
        if (eckit::StringTools::lower(values[1])[0] == 't' && eckit::StringTools::lower((values[3])) == "by") {
            eckit::Date from = tidy(ctx, values[0]);
            eckit::Date to = tidy(ctx, values[2]);
            long by = s2l(tidy(ctx, values[4]));
            values.clear();
            values.reserve((to - from) / by + 1);

            for (eckit::Date i = from; i <= to; i += by) {
                values.push_back(l2s(i.yyyymmdd()));
            }
            return;
        }
    }

    Type::expand(ctx, values);
}

void TypeDate::print(std::ostream & out) const {
    out << "TypeDate[name=" << name_ << "]";
}

static TypeBuilder<TypeDate> type("date");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
