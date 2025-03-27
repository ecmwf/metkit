/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   August 2016

#ifndef mir_LibMetkit_H
#define mir_LibMetkit_H

#include "eckit/filesystem/PathName.h"
#include "eckit/system/Library.h"

//----------------------------------------------------------------------------------------------------------------------

namespace metkit {

class LibMetkit : public eckit::system::Library {
public:

    LibMetkit();

    static eckit::PathName paramYamlFile();
    static eckit::PathName paramStaticYamlFile();
    static eckit::PathName paramIDYamlFile();
    static eckit::PathName paramMatchingYamlFile();
    static eckit::PathName shortnameContextYamlFile();
    static eckit::PathName bufrSubtypesYamlFile();

    static const LibMetkit& instance();

protected:

    const void* addr() const;

    virtual std::string version() const;

    virtual std::string gitsha1(unsigned int count) const;
};

}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------

#endif
