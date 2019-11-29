/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// #include "eckit/exception/Exceptions.h"
// #include "eckit/thread/Mutex.h"
// #include "eckit/os/Stat.h"
// #include "eckit/log/Timer.h"

#include "metkit/pointdb/PointIndex.h"
#include "metkit/grib/GribHandle.h"
#include "metkit/grib/GribIterator.h"
#include "eckit/thread/AutoLock.h"
// #include "eckit/io/StdFile.h"

// #include <eccodes.h>

using namespace eckit;

namespace metkit {
namespace pointdb {

static Mutex  mutex;
static std::set<std::string> done_;
static std::map<PathName, PointIndex*> cache_;

std::string PointIndex::cache(const metkit::grib::GribHandle& h)
{

    double lat, lon, value;

    std::string md5 = h.geographyHash();


    AutoLock<Mutex> lock(mutex);
    if (done_.find(md5) != done_.end()) {
        return md5;
    }

    eckit::PathName path(std::string("/tmp/cache/pointdb/") + md5 + ".kdtree");
    if (path.exists()) {
        done_.insert(md5);
        return md5;
    }
    PathName("/tmp/cache/pointdb/").mkdir();

    size_t v = h.getDataValuesSize();



    std::vector<Point> p;
    p.reserve(v);


    metkit::grib::GribIterator iter(h);
    size_t j = 0;

    while (iter.next(lat, lon, value)) {

        while (lon < 0)    lon += 360;
        while (lon >= 360) lon -= 360;

        // ASSERT(lat >= -90 && lat <= 90);
        // std::cout << lat << ' ' << lon << std::endl;


        p.push_back(Point(lat, lon, j));
        j++;
    }


    PathName tmp(std::string("/tmp/cache/pointdb/") + md5 + ".tmp");
    tmp.unlink();

    Tree* tree = new Tree(tmp, p.size(), 0);
    tree->build(p.begin(), p.end());

    // PathName dump(std::string("/tmp/cache/pointdb/") + md5 + ".dump");
    // StdFile f(dump, "w");
    // grib_dump_content(h, f, "debug", 0, 0);
    // f.close();

    PathName grib(std::string("/tmp/cache/pointdb/") + md5 + ".grib");
    h.write(grib);

    PathName::rename(tmp, path);

    cache_[md5] = new PointIndex(path, tree);
    done_.insert(md5);
    return md5;
}

PointIndex& PointIndex::lookUp(const std::string& md5)
{
    AutoLock<Mutex> lock(mutex);
    std::map<PathName, PointIndex*>::iterator k = cache_.find(md5);
    if (k == cache_.end()) {
        eckit::PathName path(std::string("/tmp/cache/pointdb/") + md5 + ".kdtree");

        if (!path.exists())
        {
            Log::warning() << path << " does not exists" << std::endl;
            PathName grib(std::string("/tmp/cache/pointdb/") + md5 + ".grib");
            if (grib.exists()) {
                Log::warning() << "Rebuilding index from " << grib << std::endl;
                metkit::grib::GribHandle h(grib);

                ASSERT(cache(h) == md5);
            }
        }

        Log::warning() << "Loading " << path << std::endl;
        PointIndex* p = new PointIndex(path);
        cache_[md5] = p;
        return *p;
    }
    return *(*k).second;
}

PointIndex::PointIndex(const PathName& path, PointIndex::Tree* tree):
    path_(path),
    tree_(tree) {

    if (!tree) {
        Log::info() << "Load tree " << path << std::endl;
        ASSERT(path.exists());
        tree_.reset(new Tree(path, 0, 0));
    }
}

PointIndex::~PointIndex() {
    // TODO
}

PointIndex::NodeInfo PointIndex::nearestNeighbour(double lat, double lon) {
    Point p(lat, lon, 0);

    std::map<Point, NodeInfo>::iterator k;

    {
        AutoLock<Mutex> lock(mutex_);

        k = last_.find(p);
        if (k != last_.end())
            return  (*k).second;
    }

    NodeInfo n;

    {
        Timer timer("Find nearest");
        n = tree_->nearestNeighbour(p);
    }

    {
        AutoLock<Mutex> lock(mutex_);

        if (last_.size() >= 4096)
            last_.clear();

        last_[p] = n;
    }

    return n;
}

} // namespace pointdb
} // namespace metkit
