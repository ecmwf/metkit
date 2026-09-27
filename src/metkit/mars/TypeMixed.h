/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TypeMixed.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef metkit_TypeMixed_H
#define metkit_TypeMixed_H

#include "metkit/mars/Type.h"

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeMixed : public Type {

public:  // methods

    TypeMixed(const std::string& type, Keyword key, const eckit::Value& val);
    TypeMixed(const std::string& type, Keyword key, MemFile& file);

    ~TypeMixed() noexcept override = default;

    void write(std::ofstream& file) const override;

private:  // methods

    void print(std::ostream& out) const override;
    bool expand(std::string& value, const MarsRequest& request) const override;

    std::list<std::pair<std::reference_wrapper<const Context>, std::shared_ptr<Type>>> types_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
