/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   KeySetter.h
/// @author Philipp Geier
/// @date   April 2024

#ifndef metkit_mars2grib_KeySetter_H
#define metkit_mars2grib_KeySetter_H

#include <string>

namespace metkit::mars2grib {

/**
 * Abstract interface to set eccodes/grib2 related keys
 *
 * Use internally only...
 */
class KeySetter {
public:
    virtual ~KeySetter() {};
    virtual void setValue(const std::string& key, const std::string& value) = 0;
    virtual void setValue(const std::string& key, long value)               = 0;
    virtual void setValue(const std::string& key, double value)             = 0;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib

#endif
