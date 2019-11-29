/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "pointdb/IndexFile.h"

using namespace eckit;

//========================================================================================

static Mutex mutex;
static std::map<PathName,IndexFile*> cache;

IndexFile& IndexFile::lookUp(const PathName& path)
{
    AutoLock<Mutex> lock(mutex);
    std::map<PathName,IndexFile*>::iterator k = cache.find(path);
    if(k == cache.end()) {

        if(cache.size() > 4000) {
            for(std::map<PathName,IndexFile*>::iterator j = cache.begin(); j != cache.end() ; ++j)
                (*j).second->detach();
            cache.clear();
        }

        Log::warning() << "IndexFile::lookUp opening " << path << std::endl;
        IndexFile* f = new IndexFile(path);
        f->attach();
        cache[path] = f;
        return *f;
    }
    return *(*k).second;
}
