/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0, 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
/// @author Piotr Kuchta, February 2015

#ifndef Client_H
#define Client_H

#include "eckit/runtime/Context.h"
#include "eckit/runtime/Tool.h"
#include "eckit/ecml/core/ExecutionContext.h"

class Client : public eckit::Tool {
    virtual void run();

public:
    Client(int argc, char** argv); 

    eckit::ExecutionContext& executionContext();

    bool noException() const { return noException_; }

private:
    eckit::ExecutionContext context_;
    bool noException_;
};

#endif
