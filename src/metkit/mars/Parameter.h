// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include "eckit/types/Date.h"
#include "eckit/types/Double.h"
#include "eckit/types/Time.h"
#include "eckit/utils/Translator.h"
#include "eckit/value/Value.h"

namespace eckit {
class JSON;
class MD5;
}  // namespace eckit

namespace metkit::mars {

class Type;
class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------


class Parameter {
public:  // methods

    Parameter();
    ~Parameter();

    Parameter(const std::vector<std::string>& values, const Type* = 0);
    Parameter(const Parameter&);

    Parameter& operator=(const Parameter&);
    bool operator<(const Parameter&) const;

    const std::vector<std::string>& values() const { return values_; }
    void values(const std::vector<std::string>& values);

    bool filter(const std::vector<std::string>& filter);
    bool filter(const std::string& keyword, const std::vector<std::string>& filter);
    bool matches(const std::vector<std::string>& matches) const;

    void merge(const Parameter& p);

    const Type& type() const { return *type_; }
    const std::string& name() const;

    size_t count() const;

private:  // methods

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const Parameter& p) {
        p.print(s);
        return s;
    }

private:  // members

    const Type* type_;
    std::vector<std::string> values_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
