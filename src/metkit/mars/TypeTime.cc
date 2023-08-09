/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <iomanip>
#include <regex>

#include "eckit/utils/Translator.h"
#include "eckit/types/Date.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypesFactory.h"
#include "metkit/mars/TypeTime.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeTime::TypeTime(const std::string &name, const eckit::Value& settings) :
    Type(name, settings), by_(6) {
}

TypeTime::~TypeTime() {
}

long TypeTime::seconds(std::string& value) {
    long seconds=0;
    long minutes = 0;
    long hours = 0;
    long days = 0;
    std::smatch m;
    if (std::regex_match (value, m, std::regex("^[0-9]+$"))) { // only digits
        long time = std::stol(value);
        if (time < 100) {          // cases: h, hh
            hours = time;
        } else {
            if (time < 10000) { // cases: hmm, hhmm
                hours = time/100;
                minutes = time%100;
            } else {                   // cases: hmmss, hhmmss
                hours = time/10000;
                minutes = (time/100)%100;
                seconds = time%100;
            }
        }
        if (minutes > 59 || seconds > 59) {
            std::ostringstream ss;
            ss << "Invalid time '" << value << "' ==> '" << hours << ":" << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2) << seconds << "'" << std::endl;
            throw eckit::SeriousBug(ss.str(), Here());
        }
        seconds += 60*(60*hours + minutes);
    }
    else {
        if (std::regex_match (value, m, std::regex("^([0-9]+):([0-5]?[0-9])(:[0-5]?[0-9])?$"))) {
            int count=0;
            for (int i=1; i<m.size(); i++) {
                if (m[i].matched) {
                    count++;
                    std::string aux = m[i].str();
                    if (aux[0] == ':') {
                        aux.erase(0,1);
                    }
                    seconds = seconds*60+std::stol(aux);
                }
            }
            for (; count<3; count++) {
                seconds *= 60;
            }
        }
        else {
            if (std::regex_match (value, m, std::regex("^([0-9]+d)?([0-9]+h)?([0-9]+m)?([0-9]+s)?$"))) {
                for (int i=1; i<m.size(); i++) {
                    if (m[i].matched) {
                        std::string aux = m[i].str();
                        aux.pop_back();
                        long n = std::stol(aux);
                        switch (i) {
                            case 1: days = n; break;
                            case 2: hours = n; break;
                            case 3: minutes = n; break;
                            case 4:
                            default: seconds = n;
                        }
                    }
                }
                seconds += 60*(minutes+60*(hours+24*days));
            }
        }
    }

    return seconds;
}

bool TypeTime::expand(const MarsExpandContext&, std::string &value) const {

    long time = seconds(value);
    
    long d = time/86400;
    long h = (time/3600)%24;
    long m = (time/60)%60;
    long s = time%60;

//    std::cout << value << " ==> " << d << "d" << std::setfill('0') << std::setw(2) << h << "h" << std::setfill('0') << std::setw(2) << m << "m" << std::setfill('0') << std::setw(2) << s << "s" << std::endl;

    std::ostringstream oss;
    oss << d << std::setfill('0') << std::setw(2) << h << std::setfill('0') << std::setw(2) << m << std::setfill('0') << std::setw(2) << s;
    return true;
}


void TypeTime::expand(const MarsExpandContext& ctx, std::vector<std::string>& values) const {

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

} // namespace mars
} // namespace metkit
