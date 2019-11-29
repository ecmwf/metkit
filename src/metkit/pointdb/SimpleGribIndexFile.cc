/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/pointdb/SimpleGribIndexFile.h"

using namespace eckit;

//========================================================================================
namespace metkit {
namespace pointdb {

static Mutex mutex;
static std::map<PathName, SimpleGribIndexFile*> cache;

SimpleGribIndexFile& SimpleGribIndexFile::lookUp(const PathName& path)
{
    AutoLock<Mutex> lock(mutex);
    std::map<PathName, SimpleGribIndexFile*>::iterator k = cache.find(path);
    if (k == cache.end()) {

        if (cache.size() > 4000) {
            for (std::map<PathName, SimpleGribIndexFile*>::iterator j = cache.begin(); j != cache.end() ; ++j)
                (*j).second->detach();
            cache.clear();
        }

        Log::warning() << "SimpleGribIndexFile::lookUp opening " << path << std::endl;
        SimpleGribIndexFile* f = new SimpleGribIndexFile(path);
        f->attach();
        cache[path] = f;
        return *f;
    }
    return *(*k).second;
}


} // namespace pointdb
} // namespace metkit

