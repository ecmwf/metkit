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
#include "metkit/types/TypeTime.h"
#include "eckit/parser/StringTools.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeTime::TypeTime(const std::string &name, const eckit::Value& settings) :
    Type(name, settings), by_(6) {
}

TypeTime::~TypeTime() {
}

bool TypeTime::expand(const MarsRequestContext& ctx, std::string &value) const {

    long n = 0;
    int colon = 0;

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
            n *= 10;
            n += (*j) - '0';
            break;

        case ':':
            colon++;
            break;

        default:
            // throw eckit::UserError(name_ + ": invalid time '" + value + "'");
            return false;
            break;
        }
    }

    if (colon == 2) {
        n /= 100;
    }

    if (n < 100) {
        n *= 100;
    }

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << n;
    value = oss.str();
    return true;
}


void TypeTime::expand(const MarsRequestContext& ctx, std::vector<std::string>& values) const {

    static eckit::Translator<std::string, long> s2l;
    static eckit::Translator<long, std::string> l2s;

    std::vector<std::string> newval;

    for (size_t i = 0; i < values.size(); ++i) {

        const std::string& s = values[i];

        if (eckit::StringTools::lower(s) == "to") {
            ASSERT(newval.size() > 0);
            ASSERT(i + 1 < values.size());

            long from = s2l(tidy(ctx, newval.back()));
            long to = s2l(tidy(ctx, values[i + 1]));
            long by = by_;

            if (i + 3 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                by = s2l(tidy(ctx, values[i + 3]));
                i += 2;
            }

             for (long j = from + by; j <= to; j += by) {
                newval.push_back(l2s(j));
            }

            i++;

        }
        else {
            newval.push_back(s);
        }
    }

    std::swap(values, newval);
    Type::expand(ctx, values);
}

void TypeTime::print(std::ostream &out) const {
    out << "TypeTime[name=" << name_ << "]";
}

static TypeBuilder<TypeTime> type("time");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
