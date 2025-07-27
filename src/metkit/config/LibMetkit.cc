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

#include "metkit/config/LibMetkit.h"

#include <fstream>

#include "eckit/config/Resource.h"

#include "metkit/metkit_version.h"


namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

REGISTER_LIBRARY(LibMetkit);

LibMetkit::LibMetkit() : Library("metkit") {}

const LibMetkit& LibMetkit::instance() {
    static LibMetkit libmetkit;
    return libmetkit;
}

const void* LibMetkit::addr() const {
    return this;
}

std::string LibMetkit::version() const {
    return metkit_version_str();
}

std::string LibMetkit::gitsha1(unsigned int count) const {
    std::string sha1(metkit_git_sha1());
    if (sha1.empty()) {
        return "not available";
    }

    return sha1.substr(0, std::min(count, 40u));
}

eckit::PathName LibMetkit::configFile(const std::string& filename) {
    return eckit::PathName{"~metkit/share/metkit"} / filename;
}

eckit::PathName LibMetkit::languageYamlFile() {
    return LibMetkit::configFile("language.yaml");
}
eckit::PathName LibMetkit::paramYamlFile() {
    return LibMetkit::configFile("params.yaml");
}
eckit::PathName LibMetkit::paramStaticYamlFile() {
    return LibMetkit::configFile("params-static.yaml");
}
eckit::PathName LibMetkit::shortnameContextYamlFile() {
    return LibMetkit::configFile("shortname-context.yaml");
}
eckit::PathName LibMetkit::paramIDYamlFile() {
    return LibMetkit::configFile("paramids.yaml");
}
eckit::PathName LibMetkit::paramMatchingYamlFile() {
    return LibMetkit::configFile("param-matching.yaml");
}
eckit::PathName LibMetkit::bufrSubtypesYamlFile() {
    return LibMetkit::configFile("bufr-subtypes.yaml");
}

}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------
