/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeEnum.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeEnum_H
#define metkit_TypeEnum_H

#include "metkit/mars/Type.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeEnum : public Type {

public: // methods

    TypeEnum(const std::string &name, const eckit::Value& settings);

    ~TypeEnum() override;


private: // methods
    void print(std::ostream& out) const override;
    void reset() override;
    bool expand(const MarsExpandContext& ctx, std::string& value) const override;

    std::map<std::string, std::string> mapping_;
    std::vector<std::string> values_;

    mutable std::map<std::string, std::string> cache_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace mars
} // namespace metkit

#endif
