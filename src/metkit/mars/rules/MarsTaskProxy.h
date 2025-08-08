/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date August 2025

#pragma once

#include "eckit/types/Date.h"
#include "eckit/types/Time.h"
#include "eckit/value/Value.h"

#include <string>
#include <vector>

namespace metkit::mars::rules {

class Cost;

//----------------------------------------------------------------------------------------------------------------------

/// This class exists purely to create a firewall between the internal implementations in MARS, and the expression/rules
/// engine. This engine is to be shared with, and accessible from, the FDB code, which does not have access to MARS.
/// Further, the MarsRequest class in metkit differs in implementation from that in MARS, so it is difficult to directly
/// transfer stuff here.

class MarsTaskProxy {

protected: // methods

    // Pure interface. Can only be constructed by derived classes. We should never own a MarsTaskProxy pointer.
    MarsTaskProxy() = default;
    virtual ~MarsTaskProxy() = default;

public: // methods

    virtual long getRequestValues(const std::string& key, std::vector<long>& values, bool emptyOK = false) const = 0;
    virtual long getRequestValues(const std::string& key, std::vector<std::string>& values, bool emptyOK = false) const = 0;
    virtual long getRequestValues(const std::string& key, std::vector<eckit::Date>& values, bool emptyOK = false) const = 0;
    virtual long getRequestValues(const std::string& key, std::vector<eckit::Time>& values, bool emptyOK = false) const = 0;
    virtual long getRequestValues(const std::string& key, std::vector<eckit::Value>& values, bool emptyOK = false) const = 0;

    virtual long getEnvironValues(const std::string& key, std::vector<std::string>& values, bool emptyOK = false) const = 0;
    virtual long getEnvironValues(const std::string& key, std::vector<eckit::Value>& values, bool emptyOK = false) const = 0;

    virtual bool intentOnly() const = 0;
    virtual bool authenticated() const = 0;
    virtual bool denied() const = 0;
    virtual bool beforeSchedule() const = 0;

    virtual const Cost& cost() const = 0;
};


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules
