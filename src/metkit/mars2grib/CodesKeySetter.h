/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   CodesKeySetter.h
/// @author Philipp Geier
/// @date   April 2024

#ifndef metkit_mars2grib_CodesKeySetter_H
#define metkit_mars2grib_CodesKeySetter_H

#include "metkit/codes/GribHandle.h"
#include "metkit/mars2grib/KeySetter.h"

namespace metkit::mars2grib {

class CodesKeySetter : public KeySetter {
public:
    CodesKeySetter(grib::GribHandle& h);

    virtual ~CodesKeySetter() = default;

    void setValue(const std::string& key, const std::string& value) override;
    void setValue(const std::string& key, long value) override;
    void setValue(const std::string& key, double value) override;

private:
    grib::GribHandle& handle_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib

#endif
