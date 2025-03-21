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

#include "metkit/mars/Type.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"
#include "eckit/utils/StringTools.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------
template <typename EL, typename BY>
class TypeToByList : virtual public Type {

private: // members

    const std::string by_;

public: // methods

    TypeToByList(const std::string& name, const eckit::Value& settings) :
        Type(name, settings), by_(settings.contains("by") ? settings["by"] : "1") {}

protected: // methods

    // bool ranges() const override { return true; }

    // virtual void print( std::ostream &out ) const override;
    // void expandRanges(const MarsExpandContext& ctx, std::vector<std::string>& values) const {
    void expandRanges(const MarsExpandContext& ctx, std::vector<std::string>& values) const override {

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
                    oss << name() << " list: 'to' must be preceeded by a starting value.";
                    throw eckit::BadValue(oss.str());
                }
                if (values.size() <= i+1) {
                    std::ostringstream oss;
                    oss << name() << " list: 'to' must be followed by an ending value.";
                    throw eckit::BadValue(oss.str());
                }

                EL from = s2el(tidy(ctx, values[i - 1]));
                EL to   = s2el(tidy(ctx, values[i + 1]));
                BY by   = s2by(by_);

                if (i+2 < values.size() && eckit::StringTools::lower(values[i + 2]) == "by") {
                    if (values.size() <= i+3) {
                        std::ostringstream oss;
                        oss << name() << " list: 'by' must be followed by a step size.";
                        throw eckit::BadValue(oss.str());
                    }

                    by = s2by(values[i + 3]);
                    i += 2;
                }

                if (by == BY{0}) {
                    std::ostringstream oss;
                    oss << name() + ": 'by' value " << by << " cannot be zero";
                    throw eckit::BadValue(oss.str());
                }

                EL j = from;

                if (by > BY{0}) {
                    if (to < from) {
                        std::ostringstream oss;
                        oss << name() + ": 'from' value " << from << " cannot be greater that 'to' value " << to << "";
                        throw eckit::BadValue(oss.str());
                    }
                    while (true) {
                        try {
                            j += by;
                            if (to < j) {
                                break;
                            }
                            newval.emplace_back(tidy(ctx,el2s(j)));
                        }
                        catch(...) {
                            break; /// reached an invalid value
                        }
                    }
                }
                else {
                    if (from < to) {
                        std::ostringstream oss;
                        oss << name() + ": 'from' value " << from << " cannot be lower that 'to' value " << to << "";
                        throw eckit::BadValue(oss.str());
                    }
                    while (true) {
                        try {
                            j += by;
                            if (j < to) {
                                break;
                            }
                            newval.emplace_back(tidy(ctx,el2s(j)));
                        }
                        catch(...) {
                            break; /// reached an invalid value
                        }
                    }
                }
                i++;
            }
            else {
                newval.push_back(tidy(ctx,s));
            }
        }

        std::swap(values, newval);
    }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars
