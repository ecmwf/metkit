// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   MetkitTool.h
/// @author Tiago Quintino
/// @date   Mar 2020

#pragma once

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
