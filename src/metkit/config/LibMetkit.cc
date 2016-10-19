/*
 * (C) Copyright 1996-2016 ECMWF.
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

#include "metkit/config/LibMetkit.h"

#include "eckit/config/Resource.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

// #include "metkit/api/mir_version.h"

using namespace eckit;

namespace metkit {

static Once<Mutex> local_mutex;

//----------------------------------------------------------------------------------------------------------------------

static LibMetkit libmetkit;

LibMetkit::LibMetkit() : Library("metkit") {}

const LibMetkit& LibMetkit::instance()
{
    return libmetkit;
}

const void* LibMetkit::addr() const { return this; }

std::string LibMetkit::version() const {
    // return mir_version_str();
    return "0.1";
}

std::string LibMetkit::gitsha1(unsigned int count) const {
    std::string sha1(""/*mir_git_sha1()*/);
    if (sha1.empty()) {
        return "not available";
    }

    return sha1.substr(0, std::min(count, 40u));
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace mir

