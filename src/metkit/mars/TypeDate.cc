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
#include <cctype>

#include "eckit/types/Date.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypeToByList.h"
#include "metkit/mars/TypesFactory.h"

namespace {

//----------------------------------------------------------------------------------------------------------------------

static std::array<std::string, 12> months{"jan", "feb", "mar", "apr", "may", "jun",
                                          "jul", "aug", "sep", "oct", "nov", "dec"};

std::string month(const std::string& value) {
    if (std::isdigit(value[0])) {
        eckit::Translator<std::string, int> s2i;
        int m = s2i(value);
        if (m > 12) {
            std::ostringstream oss;
            oss << value << " is not a valid month";
            throw eckit::BadValue(oss.str());
        }
        return months[m - 1];
    }

    ASSERT(value.size() >= 3);
    const auto* it = std::find(months.begin(), months.end(), eckit::StringTools::lower(value.substr(0, 3)));
    if (it == months.end()) {
        std::ostringstream oss;
        oss << value << " is not a valid month name";
        throw eckit::BadValue(oss.str());
    }
    return *it;
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

        if (tokens.size() == 2) {
            if (std::isdigit(tokens[0][0]) && tokens[0].size() > 2) {  // year-dayOfYear (e.g. 2018-23 ==> 20180123)
                eckit::Date d(s2l(tokens[0]), s2l(tokens[1]));
                return d.day();
            }
            else {  // month-day (i.e. TypeClimateDaily)
                return s2l(tokens[1]);
            }
        }
        if (tokens.size() == 1 && (!std::isdigit(value[0]) || value.size() <= 2)) {  // month (i.e. TypeClimateMonthly)
            return -1;
        }
        eckit::Date date(value);
        return date.day();
    }
    return -1;
}

bool filterByDay(const std::vector<std::string>& filter, std::vector<std::string>& values) {

    std::set<long> days;
    eckit::Translator<std::string, long> s2l;
    for (auto f : filter) {
        days.emplace(s2l(f));
    }

    for (auto v = values.begin(); v != values.end();) {
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

}  // namespace

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

TypeDate::TypeDate(const std::string& name, const eckit::Value& settings) : Type(name, settings) {

    toByList_       = std::make_unique<TypeToByList<eckit::Date, long>>(*this, settings);
    multiple_       = true;
    filters_["day"] = &filterByDay;
}

void TypeDate::pass2(const MarsExpandContext& ctx, MarsRequest& request) {
    std::vector<std::string> values = request.values(name_, true);
    if (values.size() == 1 && values[0] == "-1") {
        Type::expand(ctx, values, request);
        request.setValuesTyped(this, values);
    }
}

bool TypeDate::expand(const MarsExpandContext&, std::string& value, const MarsRequest&) const {
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

            if (tokens.size() == 2) {                                      // month-day (i.e. TypeClimateDaily)
                if (std::isdigit(tokens[0][0]) && tokens[0].size() > 2) {  // year-dayOfYear (e.g. 2018-23 ==> 20180123)
                    eckit::Date d(s2l(tokens[0]), s2l(tokens[1]));
                    value = l2s(d.yyyymmdd());
                }
                else {  // month-day (i.e. TypeClimateDaily)
                    std::string m = month(tokens[0]);
                    long d        = s2l(tokens[1]);
                    value         = m + "-" + l2s(d);
                }
            }
            else {
                if (tokens.size() == 1 &&
                    (!std::isdigit(value[0]) || value.size() <= 2)) {  // month (i.e. TypeClimateMonthly)
                    value = month(tokens[0]);
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