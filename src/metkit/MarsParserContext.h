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

#ifndef metkit_MarsParserContext_H
#define metkit_MarsParserContext_H

#include "metkit/MarsExpandContext.h"


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class MarsParserContext : public MarsExpandContext {



public:

    MarsParserContext(size_t, size_t);

private:
    size_t line_;
    size_t column_;

    virtual void info(std::ostream& out) const;

};



//----------------------------------------------------------------------------------------------------------------------


} // namespace metkit

#endif
