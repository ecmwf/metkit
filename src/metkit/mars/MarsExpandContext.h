/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#ifndef metkit_MarsExpandContext_H
#define metkit_MarsExpandContext_H

#include "eckit/memory/NonCopyable.h"

#include <iosfwd>

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

class MarsExpandContext {

    virtual void info(std::ostream&) const = 0;

public:

    virtual ~MarsExpandContext();

    friend std::ostream& operator<<(std::ostream& s, const MarsExpandContext& r) {
        r.info(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

class DummyContext : public MarsExpandContext {
    virtual void info(std::ostream&) const override;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif
