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
/// @author Manuel Fuentes
/// @author Tiago Quintino

/// @date Dec 2015

#ifndef grib_GribDataBlob_H
#define grib_GribDataBlob_H

#include "eckit/io/DataBlob.h"

#include "metkit/grib/GribMetaData.h"

namespace metkit {
namespace grib {

class GribHandle;

//----------------------------------------------------------------------------------------------------------------------

///
///
class GribDataBlob : public eckit::DataBlob {

public: // methods

    GribDataBlob(const void* data, size_t length);
    GribDataBlob(eckit::DataHandle& dh, size_t length);

	virtual ~GribDataBlob();

    virtual const eckit::Metadata& metadata() const;

private: // members

    virtual void print(std::ostream&) const;

private: // members

    grib::GribMetaData metadata_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit

#endif
