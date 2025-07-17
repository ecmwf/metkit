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

#pragma once

#include "metkit/mars/Type.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class TypeEnum : public Type {

public:  // methods

    TypeEnum(const std::string& name, const eckit::Value& settings);

    ~TypeEnum() noexcept override = default;

private:  // methods

    bool hasGroups() const override { return hasGroups_; }
    const std::vector<std::string>& group(const std::string& value) const override;

    void print(std::ostream& out) const override;
    void reset() override;
    
    bool expand(const MarsExpandContext& ctx, std::string& value, const MarsRequest& request) const override;
    int16_t find(std::string& value) const;

    std::vector<std::string> parseEnumValue(const std::string& name, const eckit::Value& val,
                                            bool uppercase, bool allowDuplicates = false);

    void addValue(const std::string& value, int16_t idx, bool allowDuplicates);
    int16_t parseValueNames(const eckit::Value& names, bool uppercase, bool allowDuplicates);

private:  // members

    bool hasGroups_ = false;

    std::vector<std::pair<std::string, std::vector<std::string>>> groups_;
    std::map<std::string, int16_t> values_; // map of acceptable values (included aliases)
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
