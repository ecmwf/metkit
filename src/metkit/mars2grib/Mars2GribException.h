/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Mars2GribException.h
/// @author Philipp Geier
/// @date   Mai 2024

#ifndef metkit_Mars2GribException_H
#define metkit_Mars2GribException_H

#include "eckit/exception/Exceptions.h"

namespace metkit::mars2grib {

class Mars2GribException : public eckit::Exception {
public:
    Mars2GribException(const std::string& reason, const eckit::CodeLocation& l = eckit::CodeLocation());
};

}

#endif
