/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/pointdb/SimpleGribIndexer.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/grib/MetFile.h"
#include "metkit/pointdb/SimpleGribDataSource.h"



namespace metkit {
namespace pointdb {

SimpleGribIndexer::SimpleGribIndexer(const eckit::PathName& path):
    path_(path) {
    scan(path);
}


std::vector<DataSourceHandle> SimpleGribIndexer::lookup(const eckit::Value& ) const {
    std::vector<DataSourceHandle> result;
    result.push_back(new SimpleGribDataSource(path_));
    return result;
}


void SimpleGribIndexer::summary(eckit::JSON& json) const {
    NOTIMP;
}


void SimpleGribIndexer::print(std::ostream& s) const {
    NOTIMP;
}



void SimpleGribIndexer::scan(const eckit::PathName& path) const {

#if 0


    // Must be large enough for TIDE and BUDG, T3999 is 62MBytes
    static long archiveGribHookBufferSize = eckit::Resource<long>("archiveGribHookBufferSize", 1024 * 1024 * 64);

    long len = 0;



    GribAccessor<unsigned long> numberOfPoints("numberOfPoints");

    PathName info = cache_.btree(path_);

    GribFileSummary summary;

    //info.unlink();
    if (!info.exists())
    {

        PathName tmp(info + ".tmp");
        if (tmp.exists())
            tmp.unlink();

        //Log::info() << "Create " << tmp << std::endl;
        IndexFile btree(tmp);

        MetFile file(path_);
        Buffer buffer(archiveGribHookBufferSize);

        unsigned long long total = path_.size();
        double read_timer = 0;
        double grib_timer = 0;
        double btree_timer = 0;
        double x;
        int count = 0;

        while ( (len = file.readSome(buffer)) != 0 )
        {

            off_t where = off_t(file.position());

            GribHandle h(buffer, len);

            FieldInfoKey k;
            FieldInfoData f(where - len , len);

            k.update(h);
            f.update(h);

            unsigned long points = numberOfPoints(h);


            grib_timer += timer.elapsed() - x;

            x = timer.elapsed();
            btree.set(k, f);
            btree_timer += timer.elapsed() - x;

            summary.add(k, points, len);

            if ((count++) % 100)
            {
                progress(where);
                double elapsed = timer.elapsed();
                if (elapsed) {
                    double speed = where / elapsed;

                    unsigned long long left = total - where;
                    double eta = left / speed;

                    Log::message() << ETA(eta) << std::endl;
                }
            }
        }
    }

#endif
}





//----------------------------------------------------------------------------------------------------------------------
} // namespace pointdb

} // namespace metkit
