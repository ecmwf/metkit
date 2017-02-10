/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Jan 2016

#ifndef grib_GribFile_H
#define grib_GribFile_H

#include "eckit/memory/NonCopyable.h"
#include "eckit/io/StdFile.h"
#include "eckit/filesystem/PathName.h"

namespace eckit { class PathName; }

namespace metkit {
namespace grib {

  class GribHandle;

//----------------------------------------------------------------------------------------------------------------------

class GribFile : private eckit::NonCopyable {

public: // methods

    /// Contructor

    GribFile(const eckit::PathName&);

    /// Destructor

    ~GribFile();

    GribHandle* next();

private: // members

    eckit::PathName path_;

    eckit::StdFile file_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit

#endif
