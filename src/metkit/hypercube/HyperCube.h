/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Aug 2017


#ifndef metkit_HyperCube_H
#define metkit_HyperCube_H

#include <iosfwd>
#include <map>
#include <memory>
#include <vector>

#include "eckit/utils/HyperCube.h"

#include "metkit/mars/MarsRequest.h"


namespace metkit {
namespace hypercube {

class Axis;

//class Sortable {
//    virtual bool operator < (const Sortable& el) const = 0;

//private:
//    metkit::mars::MarsRequest request_;

//};

class HyperCube {
public:
    HyperCube(const metkit::mars::MarsRequest&);
    ~HyperCube();

    bool contains(const metkit::mars::MarsRequest&) const;
    bool clear(const metkit::mars::MarsRequest&);

    size_t count() const;
    size_t fieldOrder(const metkit::mars::MarsRequest&, bool noholes = true) const;

    metkit::mars::MarsRequest requestOf(size_t index) const;

private:
    std::vector<Axis*> axes_;
    std::map<std::string, Axis*> axesByName_;
    std::vector<bool> set_;
    eckit::HyperCube cube_;
    size_t count_;

    int indexOf(const metkit::mars::MarsRequest&) const;

    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const HyperCube& p) {
        p.print(s);
        return s;
    }
};


template <typename T>
class HyperCubeData {
public:
    HyperCubeData(metkit::mars::MarsRequest request, std::chrono::system_clock::time_point timestamp, T data) :
        request_(request), timestamp_(timestamp), data_(data) {}

    metkit::mars::MarsRequest request() const { return request_; }
    std::chrono::system_clock::time_point timestamp() const { return timestamp_; }
    T data() const { return data_; }

    static std::vector<HyperCubeData<T>> assemble(const metkit::mars::MarsRequest& request, std::set<std::vector<HyperCubeData<T>>> datasets) {
        std::vector<HyperCubeData> out;

        HyperCube order(request);
        for (int i =0; i<order.count(); i++)
            out.push_back(HyperCubeData<T>(order.requestOf(i), std::chrono::system_clock::time_point{}, nullptr));

        for (auto datalist: datasets) {
            for (auto data: datalist) {
                size_t i = order.fieldOrder(data.request(), false);
                if (out[i].data_ == nullptr || out[i].timestamp() < data.timestamp())
                    out[i] = data;
            }
        }
        return out;
    }

    bool operator < (const HyperCubeData& el) const { return true; }

private:
    metkit::mars::MarsRequest request_;
    std::chrono::system_clock::time_point timestamp_;
    T data_;
};


}  // namespace hypercube
}  // namespace metkit


#endif
