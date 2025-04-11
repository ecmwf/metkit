/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#ifndef metkit_Parameter_H
#define metkit_Parameter_H

#include "eckit/types/Date.h"
#include "eckit/types/Double.h"
#include "eckit/types/Time.h"
#include "eckit/utils/Translator.h"
#include "eckit/value/Value.h"

namespace eckit {
class JSON;
class MD5;
}  // namespace eckit

namespace metkit {
namespace mars {

class Type;
class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------


class Parameter {
public:  // methods

    Parameter();
    ~Parameter();

    Parameter(const std::vector<std::string>& values, Type* = 0);
    Parameter(const Parameter&);

    Parameter& operator=(const Parameter&);
    bool operator<(const Parameter&) const;

    const std::vector<std::string>& values() const { return values_; }
    void values(const std::vector<std::string>& values);

    bool filter(const std::vector<std::string>& filter);
    bool filter(const std::string& keyword, const std::vector<std::string>& filter);
    bool matches(const std::vector<std::string>& matches) const;

    void merge(const Parameter& p);

    Type& type() const { return *type_; }
    const std::string& name() const;

    size_t count() const;

private:  // methods

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const Parameter& p) {
        p.print(s);
        return s;
    }

private:  // members

    Type* type_;
    std::vector<std::string> values_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
