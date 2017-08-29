/*
 * (C) Copyright 1996-2017 ECMWF.
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

#include "eckit/memory/Counted.h"
#include "eckit/types/Types.h"
#include "eckit/value/Value.h"


namespace metkit {

class MarsRequest;

//----------------------------------------------------------------------------------------------------------------------

class Type : public eckit::Counted {

public: // methods

    Type(const std::string &name, const eckit::Value& settings);

    virtual void expand(std::vector<std::string>& values) const;
    virtual bool expand(std::string& value) const;
    virtual std::string tidy(const std::string &value) const;

    virtual void setDefaults(MarsRequest& request);
    virtual void setDefaults(const std::vector<std::string>& defaults);
    virtual void check(const std::vector<std::string>& values) const;
    virtual void clearDefaults();
    virtual void reset();

    virtual void pass2(MarsRequest& request);
    virtual void finalise(MarsRequest& request);

    virtual const std::vector<std::string>& flattenValues(const MarsRequest& request);
    virtual bool flatten() const;

    virtual bool filter(const std::vector< std::string >& filter, std::vector<std::string>& values) const;
    virtual bool matches(const std::vector< std::string >& filter, const std::vector<std::string>& values) const;

    const std::string& name() const;
    const std::string& category() const;

    friend std::ostream &operator<<(std::ostream &s, const Type &x);

    virtual size_t count(const std::vector<std::string>& values) const;


public: // class methods

    static const Type &lookup(const std::string &keyword);

protected: // members

    std::string name_;
    std::string category_;

    std::vector<std::string> defaults_;
    bool flatten_;
    bool multiple_;

    std::vector<std::string> originalDefaults_;

    std::map<std::string, std::set<std::string> > only_;
    std::map<std::string, std::set<std::string> > never_;

protected: // methods

    virtual ~Type();

private: // methods

    virtual void print( std::ostream &out ) const = 0;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif
