/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeToByList.h
/// @author Emanuele Danovaro
/// @date   March 2025

#pragma once

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------
template <typename EL, typename BY>
class TypeToByList : public ITypeToByList {

private:  // members

    const Type& type_;
    const std::string by_;

public:  // methods

    TypeToByList(const Type& type, const eckit::Value& settings) :
        type_(type), by_(settings.contains("by") ? settings["by"] : "1") {}

    virtual ~TypeToByList() = default;

    void expandRanges(std::vector<std::string>& values, const MarsRequest& request) const override {

        if (values.size() == 1) {
            return;
        }

        static eckit::Translator<std::string, EL> s2el;
        static eckit::Translator<std::string, BY> s2by;
        static eckit::Translator<EL, std::string> el2s;

        std::vector<std::string> newval;

        for (size_t i = 0; i < values.size(); ++i) {

            const std::string& s = values[i];

            if (eckit::StringTools::lower(s) == "to" || eckit::StringTools::lower(s) == "t0") {
                // TimeUnit unit;

                if (newval.size() == 0) {
                    std::ostringstream oss;
                    oss << type_.name() << " list: 'to' must be preceeded by a starting value.";
                    throw eckit::BadValue(oss.str());
                }
                if (values.size() <= i + 1) {
                    std::ostringstream oss;
                    oss << type_.name() << " list: 'to' must be followed by an ending value.";
                    throw eckit::BadValue(oss.str());
                }

                EL from          = s2el(type_.tidy(values[i - 1], request));
                std::string to_s = type_.tidy(values[i + 1], request);
                EL to            = s2el(to_s);
                BY by            = s2by(by_);

                if (i + 2 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                    if (values.size() <= i + 3) {
                        std::ostringstream oss;
                        oss << type_.name() << " list: 'by' must be followed by a step size.";
                        throw eckit::BadValue(oss.str());
                    }

                    by = s2by(values[i + 3]);
                    i += 2;
                }
                i++;

                if (by == BY{0}) {
                    std::ostringstream oss;
                    oss << type_.name() + ": 'by' value " << by << " cannot be zero";
                    throw eckit::BadValue(oss.str());
                }
                if (from < to && by < BY{0}) {
                    std::ostringstream oss;
                    oss << type_.name() << ": impossible to define a sequence starting from " << from << " to " << to
                        << " with step " << by;
                    throw eckit::BadValue(oss.str());
                }
                bool addBy = (from < to && by > BY{0}) || (from > to && by < BY{0});
                if (from == to) {
                    continue;
                }

                EL j = from;
                while (j != to) {
                    std::string j_s;
                    try {
                        if (addBy) {
                            j += by;
                        }
                        else {
                            j -= by;
                        }
                        j_s = type_.tidy(el2s(j), request);
                    }
                    catch (...) {
                        break;  /// reached an invalid value
                    }

                    if (((from < to && j > to) || (from > to && j < to)) && j != to && j_s != to_s) {
                        break;
                    }
                    newval.push_back(j_s);
                }
            }
            else {
                newval.push_back(type_.tidy(s, request));
            }
        }

        std::swap(values, newval);
    }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
