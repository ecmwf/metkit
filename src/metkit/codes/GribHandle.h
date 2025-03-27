/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/memory/NonCopyable.h"

struct grib_handle;

namespace eckit {
class DataHandle;
}

namespace metkit {
namespace grib {

class GribMetaData;
class GribAccessorBase;
class GribMutatorBase;

//----------------------------------------------------------------------------------------------------------------------

void codes_call(int code, const char* msg, const eckit::CodeLocation& where);

#define CODES_CALL(a) metkit::grib::codes_call(a, #a, Here())

//----------------------------------------------------------------------------------------------------------------------

class GribHandle : private eckit::NonCopyable {

public:  // types

    /// constructor from file path, creates grib_handle and takes ownership
    /// @note currently this only handles local paths
    explicit GribHandle(const eckit::PathName&);

    /// constructor taking ownership of a grib_handle pointer
    GribHandle(grib_handle*);

    /// constructor not taking
    GribHandle(grib_handle&);

    /// constructor creating a grib_handle from a DataHandle
    explicit GribHandle(eckit::DataHandle&);

    // Constructor creating a grib_handle from a DataHandle starting from a given offset
    explicit GribHandle(eckit::DataHandle&, eckit::Offset);

    /// destructor will delete the grib_handle if we own it
    ~GribHandle() noexcept(false);

public:  // methods

    size_t length() const;

    // std::string gridType() const;

    std::string geographyHash() const;

    GribHandle* clone() const;

    // std::string shortName() const;

    // size_t numberOfPoints() const;

    size_t getDataValuesSize() const;
    double* getDataValues(size_t&) const;
    void getDataValues(double*, const size_t&) const;

    void setDataValues(const double*, size_t);

    size_t write(eckit::DataHandle&) const;
    size_t write(eckit::Buffer&) const;
    void write(const eckit::PathName&, const char* mode = "w") const;
    void dump(const eckit::PathName&, const char* mode = "debug") const;

    // double latitudeOfFirstGridPointInDegrees()  const;
    // double longitudeOfFirstGridPointInDegrees() const;
    // double latitudeOfLastGridPointInDegrees()   const;
    // double longitudeOfLastGridPointInDegrees()  const;

    bool hasKey(const char*) const;

    operator const grib_handle*() const { return handle_; }

protected:  // methods

    friend class GribAccessorBase;
    friend class GribMutatorBase;
    friend class GribIterator;

    /// To be used by friends since this is rather dangerous
    /// Don't delete this pointer, use with care :)
    /// @returns the raw grib_handle so client code can call grib directly

    grib_handle* raw() const { return handle_; }

    /// Client code shouldn't care if GRIB edition
    long edition() const;

private:  // members

    grib_handle* handle_;

    bool owned_;
};

//------------------------------------------------------------------------------------------------------

}  // namespace grib
}  // namespace metkit
