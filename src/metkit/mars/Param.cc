// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "metkit/mars/Param.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/persist/DumpLoad.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------


Param::Param(const std::string& s) {
    Tokenizer parse(".");
    std::vector<std::string> result;

    parse(s, result);
    switch (result.size()) {
        case 1:
            value_ = atol(result[0].c_str());
            table_ = 0;
            break;

        case 2:
            value_ = atol(result[0].c_str());
            table_ = atol(result[1].c_str());
            break;

        default:
            throw eckit::BadValue("Invalid param");
    }
}

Param::operator std::string() const {
    std::ostringstream os;
    os << *this;
    return os.str();
}

void Param::print(std::ostream& s) const {
    if (table_)
        s << value_ << '.' << table_;
    else
        s << value_;
}

void Param::dump(DumpLoad& a) const {
    a.dump(value_);
    a.dump(table_);
}

void Param::load(DumpLoad& a) {
    a.load(value_);
    a.load(table_);
}

long Param::paramId() const {
    size_t t = table_ == 0 ? value_ / 1000 : table_;
    return (t == 128 ? 0 : t * 1000) + value_ % 1000;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
