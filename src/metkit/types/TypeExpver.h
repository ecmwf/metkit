/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeExpver.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeExpver_H
#define metkit_TypeExpver_H

#include "metkit/types/Type.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class TypeExpver : public Type {

public: // methods

    TypeExpver(const std::string &name, const eckit::Value& settings);

    virtual ~TypeExpver();

virtual bool expand(std::string& value) const ;

private: // methods

    virtual void print( std::ostream &out ) const;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
