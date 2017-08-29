/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeRange.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeRange_H
#define metkit_TypeRange_H

#include "metkit/types/TypeToByList.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class TypeRange : public TypeToByList {

public: // methods

    TypeRange(const std::string &name, const eckit::Value& settings);

    virtual ~TypeRange();

private: // methods

    virtual void print( std::ostream &out ) const;
    virtual bool expand(std::string& value) const;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
