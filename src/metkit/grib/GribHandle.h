/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef grib_GribHandle_H
#define grib_GribHandle_H

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/memory/Owned.h"

struct grib_handle;

namespace eckit {
    class DataHandle;
}

namespace metkit {
namespace grib {

    class GribMetaData;
    class GribDataBlob;
    class GribAccessorBase;
    class GribMutatorBase;

//----------------------------------------------------------------------------------------------------------------------

void grib_call( int code, const char* msg, const eckit::CodeLocation& where );

#define GRIB_CALL(a) grib::grib_call(a, #a, Here())

//----------------------------------------------------------------------------------------------------------------------

class GribHandle : public eckit::Owned {

public: // types

   /// constructor from file path, creates grib_handle and takes ownership
   /// @note currently this only handles local paths
   explicit GribHandle(const eckit::PathName&);

    /// constructor taking ownership of a grib_handle pointer
    GribHandle(grib_handle*);

    /// constructor not taking
    GribHandle(grib_handle&);

   /// constructor creating a grib_handle from a buffer
   explicit GribHandle(const eckit::Buffer&, bool copy = true);

   /// destructor will delete the grib_handle if we own it
   ~GribHandle();

public: // methods

   /// @returns a new GribDataBlob
   GribDataBlob* message() const;

   std::string gridType() const;

   std::string geographyHash() const;

   GribHandle* clone() const;

   std::string shortName() const;

   size_t numberOfPoints() const;

   size_t  getDataValuesSize() const;
   double* getDataValues(size_t&) const;
   void    getDataValues(double*, const size_t &) const;

   void setDataValues(const double*, size_t);

   void   write( eckit::DataHandle& );
   size_t write( eckit::Buffer& );

   double latitudeOfFirstGridPointInDegrees()  const;
   double longitudeOfFirstGridPointInDegrees() const;
   double latitudeOfLastGridPointInDegrees()   const;
   double longitudeOfLastGridPointInDegrees()  const;

   bool hasKey(const char*) const;

protected: // methods

   friend class GribDataBlob;
   friend class GribMetaData;
   friend class GribAccessorBase;
   friend class GribMutatorBase;

   /// To be used by friends since this is rather dangerous
   /// Don't delete this pointer, use with care :)
   /// @returns the raw grib_handle so client code can call grib directly
   grib_handle* raw() const { return handle_; }

   /// Client code shouldn't care if GRIB edition
   long edition() const;

private: // members

   grib_handle* handle_;

   bool owned_;

};

//------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit

#endif
