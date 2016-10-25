/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeToByList.h"

#include "eckit/utils/Translator.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

TypeToByList::TypeToByList(const std::string &name, const eckit::Value& settings) :
    TypeInteger(name, settings),
    by_(settings["by"]) {
}

TypeToByList::~TypeToByList() {
}

void TypeToByList::print(std::ostream &out) const {
    out << "TypeToByList[name=" << name_ << "]";
}

void TypeToByList::expand(std::vector<std::string>& values) const {

    static eckit::Translator<std::string, long> s2l;
    static eckit::Translator<long, std::string> l2s;

    if (values.size() == 3) {
        if (values[1] == "to") {
            long from = s2l(values[0]);
            long to = s2l(values[2]);
            long by = by_;
            values.clear();
            values.reserve((to - from) / by + 1);
            for (long i = from; i <= to; i += by) {
                values.push_back(l2s(i));
            }
            return;
        }
    }

    if (values.size() == 5) {
        if (values[1] == "to" && values[3] == "by") {
            if (values[1] == "to") {
                long from = s2l(values[0]);
                long to = s2l(values[2]);
                long by = s2l(values[4]);
                values.clear();
                values.reserve((to - from) / by + 1);

                for (long i = from; i <= to; i += by) {
                    values.push_back(l2s(i));
                }
                return;
            }
        }
    }

    TypeInteger::expand(values);
}

static TypeBuilder<TypeToByList> type("to-by-list");

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
