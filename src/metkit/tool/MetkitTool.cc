/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/system/Library.h"

#include "metkit/tool/MetkitTool.h"
#include "metkit/config/LibMetkit.h"

using namespace eckit;
using namespace eckit::option;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

static MetkitTool* instance_ = nullptr;

MetkitTool::MetkitTool(int argc, char** argv) : eckit::Tool(argc, argv, "METKIT_HOME") {
    ASSERT(instance_ == nullptr);
    instance_ = this;

    options_.push_back(new SimpleOption<bool>("version", "Prints the version and exits"));
    options_.push_back(new SimpleOption<bool>(
        "porcelain", "Stable output that can be used as input to other tools"));
}

static void usage(const std::string& tool) {
    ASSERT(instance_);
    instance_->usage(tool);
}

void MetkitTool::run() {
    LOG_DEBUG_LIB(LibMetkit) << "MetkitTool::run()" << std::endl;

    CmdArgs args(&metkit::usage, options_, numberOfPositionalArguments(),
                 minimumPositionalArguments());

    if (args.has("version")) {
        auto& log = eckit::Log::info();

        using eckit::system::Library;
        for (const auto& lib_name : Library::list()) {
            auto& lib = Library::lookup(lib_name);
            log << lib.name() << " " << lib.version() << " git-sha1:" << lib.gitsha1(8)
                << " home:" << lib.libraryHome() << std::endl;
        }
        return;  //< stops processing
    }

    init(args);
    execute(args);
    finish(args);
}

void MetkitTool::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << "[-h] [...options...]" << std::endl;
}

void MetkitTool::init(const eckit::option::CmdArgs& args) {
    LOG_DEBUG_LIB(LibMetkit)  << "MetkitTool::init()" << std::endl;
    args.get("porcelain", porcelain_);
}

void MetkitTool::finish(const eckit::option::CmdArgs& args) {
    LOG_DEBUG_LIB(LibMetkit)  << "MetkitTool::finish()" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

MetkitToolException::MetkitToolException(const std::string& w) : Exception(w) {}

MetkitToolException::MetkitToolException(const std::string& w, const eckit::CodeLocation& l) :
    Exception(w, l) {}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit
