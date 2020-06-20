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


#ifndef mars_client_Reader_H
#define mars_client_Reader_H

#include <iosfwd>
#include <memory>

#include "eckit/memory/NonCopyable.h"
#include "eckit/io/PeekHandle.h"
#include "metkit/codes/Message.h"


namespace eckit {
class DataHandle;
class PathName;
class Offset;
};

namespace metkit {
namespace codes {

class Message;
class Splitter;

class ReaderFilter {
public:
    virtual bool operator()(const Message&) const = 0;
    static ReaderFilter& none();
};

class Reader : public eckit::NonCopyable {
public:

    Reader(eckit::DataHandle*, bool opened=false, const ReaderFilter& = ReaderFilter::none());
    Reader(eckit::DataHandle&, bool opened=false, const ReaderFilter& = ReaderFilter::none());

    Reader(const eckit::PathName&, const ReaderFilter& = ReaderFilter::none());

    ~Reader();

    Message next();
    eckit::Offset position();

private:

    bool opened_;
    std::unique_ptr<Splitter> splitter_;

    const ReaderFilter& filter_;
    eckit::PeekHandle handle_;

    void init();
    void print(std::ostream &) const; // Change to virtual if base class

    friend std::ostream &operator<<(std::ostream &s, const Reader &p) {
        p.print(s);
        return s;
    }

};


}  // namespace codes
}  // namespace metkit


#endif
