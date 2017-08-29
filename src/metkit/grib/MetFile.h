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

#ifndef grib_MetFile_H
#define grib_MetFile_H

#include "eckit/io/DataHandle.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"

// Forward declarations

namespace eckit { class Buffer; }
namespace eckit { class PathName; }
namespace eckit { class CircularBuffer; }
namespace eckit { class ResizableBuffer; }

namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

/// Previously existed in mars-server code as marslib/EmosFile

class MetFile : private eckit::NonCopyable {

public: // methods

    /// Contructor

    MetFile(const eckit::PathName&, bool buffered = true);

    /// Contructor, does not take ownership of eckit::DataHandle

    MetFile( eckit::DataHandle& dh );

    /// Destructor

    ~MetFile();

public: // methods

    long read(eckit::Buffer&);
    long read(eckit::CircularBuffer&);
    long read(eckit::ResizableBuffer&);

    // Don't fail if buffer is too small
    // FIXME: Why are we not failing if the buffer is too small? As is, the
    // caller is required to check whether the buffer was sufficiently large
    long readSome(eckit::Buffer&);

    eckit::Offset position();

    void rewind();
    void seek(const eckit::Offset&);

private: // members

    eckit::ScopedPtr<eckit::DataHandle> handle_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit

#endif
