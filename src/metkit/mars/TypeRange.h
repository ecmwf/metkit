/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeToByListQuantile.h
/// @author Emanuele Danovaro
/// @date   February 2022

#pragma once

#include "metkit/mars/Type.h"
#include "metkit/mars/TypeToByList.h"

namespace metkit::mars {

class StepRange;
//----------------------------------------------------------------------------------------------------------------------

class ExtendedTime : public eckit::Time {
public:
    ExtendedTime(long seconds = 0) : Time(seconds, true) {}
    ExtendedTime(const std::string& time) : Time(time, true) {}
};

//----------------------------------------------------------------------------------------------------------------------

class TypeRange : virtual public Type, public TypeToByList<StepRange, ExtendedTime> {

public: // methods

    TypeRange(const std::string &name, const eckit::Value& settings);

    virtual ~TypeRange() override;

private: // methods

    virtual void print( std::ostream &out ) const override;
    virtual bool expand(const MarsExpandContext& ctx, std::string& value) const override;

    StepRange parse(std::string& value) const;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars
