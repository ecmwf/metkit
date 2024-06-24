/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   ValueMapSetter.h
/// @author Philipp Geier
/// @date   April 2024

#ifndef metkit_mars2grib_ValueMapSetter_H
#define metkit_mars2grib_ValueMapSetter_H

#include "eckit/value/Value.h"
#include "metkit/mars2grib/KeySetter.h"

#include <deque>

namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

class ValueMapSetter : public KeySetter {
public:
    ValueMapSetter(eckit::ValueMap& map);

    virtual ~ValueMapSetter() = default;

    void setValue(const std::string& key, const std::string& value) override;
    void setValue(const std::string& key, long value) override;
    void setValue(const std::string& key, double value) override;

    void setValue(const std::string& key, NullOrMissing) override;

    void print(std::ostream& os) const override;

private:
    eckit::ValueMap& map_;
};


//----------------------------------------------------------------------------------------------------------------------

class OrderedValueMapSetter : public KeySetter {
public:
    OrderedValueMapSetter(eckit::ValueMap& map, std::deque<std::string>& keys);

    virtual ~OrderedValueMapSetter() = default;

    void setValue(const std::string& key, const std::string& value) override;
    void setValue(const std::string& key, long value) override;
    void setValue(const std::string& key, double value) override;

    void setValue(const std::string& key, NullOrMissing) override;

    void print(std::ostream& os) const override;

private:
    eckit::ValueMap& map_;
    std::deque<std::string>& keys_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib

#endif
