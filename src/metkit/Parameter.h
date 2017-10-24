/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "eckit/value/Value.h"
#include "eckit/utils/Translator.h"

namespace eckit {
class JSON;
class MD5;
}

namespace metkit {

class Type;
class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------


class Parameter {
public:

    Parameter();
    ~Parameter();

    Parameter(const std::vector<std::string>& values, Type* = 0);
    Parameter(const Parameter&);
    Parameter& operator=(const Parameter&);
    bool operator<(const Parameter&) const;

    const std::vector< std::string >& values() const { return values_; }
    void values(const std::vector< std::string >& values);

    bool filter(const std::vector< std::string >& filter);
    bool matches(const std::vector< std::string >& matches) const;

    Type& type() const { return *type_; }
    const std::string& name() const;

    size_t count() const;

private:

    Type* type_;
    std::vector<std::string> values_;
};


//----------------------------------------------------------------------------------------------------------------------


} // namespace metkit

#endif
