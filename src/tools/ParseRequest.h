// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

#include "eckit/filesystem/PathName.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/tool/MetkitTool.h"

using eckit::PathName;
using eckit::option::CmdArgs;
using eckit::option::SimpleOption;
using metkit::MetkitTool;

//----------------------------------------------------------------------------------------------------------------------

class ParseRequest : public MetkitTool {
public:

    ParseRequest(int argc, char** argv, bool convertToGrib2 = false) : MetkitTool(argc, argv), grib2_(convertToGrib2) {
        options_.push_back(new SimpleOption<bool>("json", "Format request in json, default = false"));
        options_.push_back(new SimpleOption<bool>("compact", "Compact output, default = false"));
        options_.push_back(new SimpleOption<bool>("block", "Stable output that can be used as input to other tools. One keyword per line"));
    }

    virtual ~ParseRequest() {}

private:  // methods

    int minimumPositionalArguments() const { return 1; }

    void process(const eckit::PathName& path);

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual void init(const CmdArgs& args);

    virtual void usage(const std::string& tool) const;

private:  // members

    bool json_    = false;
    bool compact_ = false;
    bool block_   = false;
    bool grib2_   = false;
};
