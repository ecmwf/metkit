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

#include <mutex>

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
    std::map<std::string, uint16_t>::const_iterator find(const std::string& value) const;

    std::vector<std::string> parseEnumValue(const eckit::Value& val, bool allowDuplicates = false) const;

    void addValue(const std::string& value, uint16_t idx, bool allowDuplicates) const;
    uint16_t parseValueNames(const eckit::Value& names, bool allowDuplicates) const;

    void readValuesFile() const;

private:  // members

    std::string valuesFile_;

    bool uppercase_         = false;
    mutable bool hasGroups_ = false;
    mutable std::vector<std::pair<std::string, std::vector<std::string>>> groups_;
    mutable std::map<std::string, uint16_t> values_;  // map of acceptable values (included aliases)

    mutable std::once_flag readValues_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
