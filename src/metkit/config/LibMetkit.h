// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   August 2016

#pragma once

#include "eckit/filesystem/PathName.h"
#include "eckit/system/Library.h"

//----------------------------------------------------------------------------------------------------------------------

namespace metkit {

class LibMetkit : public eckit::system::Library {
public:

    LibMetkit();

    static eckit::PathName configFile(const std::string& filename);

    static uint16_t binaryFilesVersion() { return 2; }

    static eckit::PathName languageYamlFile();
    static std::vector<eckit::PathName> modifiersYamlFiles();
    static eckit::PathName paramYamlFile();
    static eckit::PathName paramStaticYamlFile();
    static eckit::PathName paramIDYamlFile();
    static eckit::PathName paramsBinaryFile();
    static eckit::PathName paramMatchingYamlFile();
    static eckit::PathName shortnameContextYamlFile();
    static eckit::PathName bufrSubtypesYamlFile();

    static const LibMetkit& instance();

protected:

    const void* addr() const;

    virtual std::string version() const;

    virtual std::string gitsha1(unsigned int count) const;
};

}  // namespace metkit

//----------------------------------------------------------------------------------------------------------------------
