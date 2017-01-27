/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeToByList.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeToByList_H
#define metkit_TypeToByList_H

#include "metkit/types/TypeInteger.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class TypeToByList : public TypeInteger {

public: // methods

    TypeToByList(const std::string &name, const eckit::Value& settings);

    virtual ~TypeToByList();

private: // methods

    virtual void print( std::ostream &out ) const;
    virtual void expand(std::vector<std::string>& values) const;

    long by_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
