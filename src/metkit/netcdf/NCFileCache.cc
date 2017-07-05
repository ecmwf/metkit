/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "metkit/netcdf/NCFileCache.h"
#include "metkit/netcdf/NCFile.h"

namespace metkit {
namespace netcdf {

NCFileCache::NCFileCache() {

}

NCFileCache::~NCFileCache() {
    for (std::map<std::string, NCFile *>::iterator j = files_.begin(); j != files_.end(); ++j)
    {
        delete (*j).second;
    }
}

NCFile &NCFileCache::lookUp(const std::string &path) {
    std::map<std::string, NCFile *>::iterator j = files_.find(path);
    if (j == files_.end()) {
        NCFile *f = new NCFile(path);
        files_[path] = f;
        return *f;
    }
    return *((*j).second);
}

}
}
