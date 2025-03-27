/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   MetkitTool.h
/// @author Tiago Quintino
/// @date   Mar 2020

#ifndef metkit_MetkitTool_H
#define metkit_MetkitTool_H

#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Tool.h"

namespace eckit {
namespace option {
class Option;
class CmdArgs;
}  // namespace option
}  // namespace eckit

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class MetkitTool : public eckit::Tool {

public:  // methods

    MetkitTool(int argc, char** argv);

    virtual void usage(const std::string& tool) const;

protected:  // methods

    using options_t = std::vector<eckit::option::Option*>;

protected:  // members

    options_t options_;

    bool porcelain_ = false;

private:  // methods

    virtual void init(const eckit::option::CmdArgs& args);
    virtual void execute(const eckit::option::CmdArgs& args) = 0;
    virtual void finish(const eckit::option::CmdArgs& args);

    virtual int numberOfPositionalArguments() const { return -1; }
    virtual int minimumPositionalArguments() const { return -1; }

    virtual void run();
};

//----------------------------------------------------------------------------------------------------------------------


class MetkitToolException : public eckit::Exception {
public:

    MetkitToolException(const std::string&);
    MetkitToolException(const std::string&, const eckit::CodeLocation&);
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit

#endif
