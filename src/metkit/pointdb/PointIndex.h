/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef PointIndex_H
#define PointIndex_H

// #include <cmath>
#include <memory>

// #include "eckit/eckit.h"
// #include "eckit/filesystem/PathName.h"


#include "eckit/container/KDTree.h"
#include "eckit/geometry/Point3.h"

namespace metkit {
namespace grib {
class GribHandle;
}

namespace pointdb {


struct LLPoint2 : public eckit::geometry::Point3 {

    double lat_;
    double lon_;
    size_t payload_;


    double lat() const { return lat_; }
    double lon() const { return lon_; }


    size_t payload() const { return payload_; }

    const LLPoint2& point() const { return *this; }


    LLPoint2() : eckit::geometry::Point3() {}

    LLPoint2(double lat, double lon, size_t index) : eckit::geometry::Point3(), lat_(lat), lon_(lon) {

        // See http://en.wikipedia.org/wiki/Geodetic_system#From_geodetic_to_ECEF
        payload_  = index;
        double& X = x_[0];
        double& Y = x_[1];
        double& Z = x_[2];

        double h = 0;  // Altitude

        const double earthRadius = 6378137.0;
        double a                 = earthRadius;  // 6378137.0 ; //  WGS84 semi-major axis

        double e2 = 0;  // 6.69437999014E-3; // WGS84 first numerical eccentricity sqared

        double phi    = lat / 180.0 * M_PI;
        double lambda = lon / 180.0 * M_PI;

        double cos_phi    = cos(phi);
        double sin_phi    = sin(phi);
        double cos_lambda = cos(lambda);
        double sin_lambda = sin(lambda);

        double N_phi = a / sqrt(1 - e2 * sin_phi * sin_phi);

        X = (N_phi + h) * cos_phi * cos_lambda;
        Y = (N_phi + h) * cos_phi * sin_lambda;
        Z = (N_phi * (1 - e2) + h) * sin_phi;
    }

    friend std::ostream& operator<<(std::ostream& s, const LLPoint2& p) {
        s << '(' << p.lat_ << "," << p.lon_ << ' ' << p.payload_ << ')';
        return s;
    }
};


struct PointIndexTraits {
    using Point   = LLPoint2;
    using Payload = size_t;
};

class PointIndex {
public:

    typedef eckit::KDTreeMapped<PointIndexTraits> Tree;
    typedef Tree::Point Point;
    typedef Tree::NodeInfo NodeInfo;

    NodeInfo nearestNeighbour(double lat, double lon);

    static PointIndex& lookUp(const std::string& md5);
    static std::string cache(const metkit::grib::GribHandle& h);

    static eckit::PathName cachePath(const std::string& dir, const std::string& name);

private:

    PointIndex(const eckit::PathName&, Tree* tree = 0);
    ~PointIndex();

    eckit::PathName path_;
    std::unique_ptr<Tree> tree_;

    std::map<Point, NodeInfo> last_;
    eckit::Mutex mutex_;
};

}  // namespace pointdb
}  // namespace metkit

#endif
