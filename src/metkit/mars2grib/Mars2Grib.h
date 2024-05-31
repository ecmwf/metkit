/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Mars2Grib.h
/// @author Philipp Geier
/// @date   April 2024

#ifndef metkit_Mars2Grib_H
#define metkit_Mars2Grib_H

#include "metkit/mars2grib/KeySetter.h"

namespace metkit::mars2grib {
/**
 * Reads specific mars keys
 */
static void convertMars2Grib(const eckit::ValueMap&, KeySetter&);


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib

#endif
