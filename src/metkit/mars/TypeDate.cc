/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/mars/TypeDate.h"

#include <algorithm>
#include <array>

#include "eckit/types/Date.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "eckit/utils/StringTools.h"
#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

#include "metkit/mars/MarsExpandContext.h"

namespace {
static std::array<std::string, 12> months{"jan", "feb", "mar", "apr", "may", "jun",
                                            "jul", "aug", "sep", "oct", "nov", "dec"};

int month(const std::string& value) {
    if (value.size() == 3) {
        auto it = std::find(months.begin(), months.end(), eckit::StringTools::lower(value));
        if (it == months.end()) {
            std::ostringstream oss;
            oss << value << " is not a valid month short name";
            throw eckit::BadValue(oss.str());
        }
        return it - months.begin() + 1;
    }

    eckit::Translator<std::string, int> s2i;
    return s2i(value);
}

long day(std::string& value) {
    if (value.empty()) {
        return -1;
    }
    eckit::Translator<std::string, long> s2l;
    if (value[0] == '0' || value[0] == '-') {
        long n = s2l(value);
        if (n <= 0) {
            eckit::Date now(n);
            return now.day();
        }
    }
    else {
        eckit::Tokenizer t("-");
        std::vector<std::string> tokens = t.tokenize(value);

        if (tokens.size() == 2) {  // month-day (i.e. TypeClimateDaily)
            int m  = month(tokens[0]);
            return s2l(tokens[1]);
        }
        else {
            if (tokens.size() == 1 && tokens[0].size() <= 3) {  // month (i.e. TypeClimateMonthly)
                return -1;
            }
            else {
                eckit::Date date(value);
                return date.day();
            }
        }
    }
    return -1;
}

bool filterByDay(const std::vector<std::string>& filter, std::vector<std::string>& values) {

    std::set<long> days;
    eckit::Translator<std::string, long> s2l;
    for (auto f : filter) {
        days.emplace(s2l(f));
    }

    for (auto v=values.begin(); v != values.end();) {
        long d = day(*v);
        if (d != -1) {
            auto it = days.find(d);
            if (it != days.end()) {
                v++;
                continue;
            }
        }
        v = values.erase(v);
    }

    return !values.empty();
}
}
    
namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeDate::TypeDate(const std::string& name, const eckit::Value& settings) : Type(name, settings) {

    DummyContext ctx;
    toByList_ = std::make_unique<TypeToByList<eckit::Date, long>>(*this, settings);

    multiple_ = true;

    filters_["day"] = &filterByDay;
}

TypeDate::~TypeDate() {}

void TypeDate::pass2(const MarsExpandContext& ctx, MarsRequest& request) {
    std::vector<std::string> values = request.values(name_, true);
    Type::expand(ctx, values);
    request.setValuesTyped(this, values);
}

bool TypeDate::expand(const MarsExpandContext& ctx, std::string& value) const {
    if (!value.empty()) {
        eckit::Translator<std::string, long> s2l;
        eckit::Translator<long, std::string> l2s;
        if (value[0] == '0' || value[0] == '-') {
            long n = s2l(value);
            if (n <= 0) {
                eckit::Date now(n);
                value = l2s(now.yyyymmdd());
            }
        }
        else {
            eckit::Tokenizer t("-");
            std::vector<std::string> tokens = t.tokenize(value);

            if (tokens.size() == 2) {  // month-day (i.e. TypeClimateDaily)
                int m  = month(tokens[0]);
                long d = s2l(tokens[1]);

                value = l2s(100 * m + d);
            }
            else {
                if (tokens.size() == 1 && tokens[0].size() <= 3) {  // month (i.e. TypeClimateMonthly)
                    int m = month(tokens[0]);
                    value = l2s(m);
                }
                else {
                    eckit::Date date(value);
                    value = l2s(date.yyyymmdd());
                }
            }
        }
    }
    return true;
}

void TypeDate::print(std::ostream& out) const {
    out << "TypeDate[name=" << name_ << "]";
}

static TypeBuilder<TypeDate> type("date");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
