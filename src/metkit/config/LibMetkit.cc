// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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

bool LibMetkit::isMultiParam() {
    static bool multiParamValues = eckit::Resource<bool>("metkitMultiParamValues;$METKIT_MULTI_PARAM_VALUES", false);
    return multiParamValues;
}

eckit::PathName LibMetkit::configFile(const std::string& filename) {
    return eckit::PathName{"~metkit/share/metkit"} / filename;
}

eckit::PathName LibMetkit::languageYamlFile() {
    return LibMetkit::configFile("language.yaml");
}
std::vector<eckit::PathName> LibMetkit::modifiersYamlFiles() {
    return std::vector<eckit::PathName>{LibMetkit::configFile("modifiers.yaml"),
                                        LibMetkit::configFile("reportype.yaml")};
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
eckit::PathName LibMetkit::paramsBinaryFile() {
    return LibMetkit::configFile(isMultiParam() ? "params-multi.bin" : "params.bin");
}
eckit::PathName LibMetkit::paramMatchingYamlFile() {
    return LibMetkit::configFile("param-matching.yaml");
}
eckit::PathName LibMetkit::bufrSubtypesYamlFile() {
    return LibMetkit::configFile("bufr-subtypes.yaml");
}

}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------
