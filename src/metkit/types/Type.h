/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Type.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_Type_H
#define metkit_Type_H

#include <string>

#include "eckit/memory/NonCopyable.h"
#include "eckit/types/Types.h"
#include "eckit/value/Value.h"


namespace metkit {

class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------

class Type : private eckit::NonCopyable {

public: // methods

    Type(const std::string &name, const eckit::Value& settings);

    virtual ~Type();

    virtual std::string tidy(const std::string &value) const ;

    virtual void expand(std::vector<std::string>& values) const;
    virtual void setDefaults(MarsRequest& request) const;
    virtual void setDefaults(const std::vector<std::string>& defaults);
    virtual void clearDefaults();

    virtual void flattenValues(const MarsRequest& request, std::vector<std::string>& values);
    virtual bool flatten() const;

    friend std::ostream &operator<<(std::ostream &s, const Type &x);

public: // class methods

    static const Type &lookup(const std::string &keyword);


protected: // members

    std::string name_;
    std::vector<std::string> defaults_;
    bool flatten_;

protected: // methods


private: // methods

    virtual void print( std::ostream &out ) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
