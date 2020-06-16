/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/Reader.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/codes/Message.h"

#include <eccodes.h>

namespace metkit {
namespace codes {

class NoFilter : public ReaderFilter {
    virtual bool operator()(const Message&) const {
        return true;
    }
};


ReaderFilter& ReaderFilter::none() {
    static NoFilter nofilter;
    return nofilter;
}

Reader::Reader(eckit::DataHandle* h, bool opened, const ReaderFilter& filter):
    handle_(h),
    opened_(opened),
    filter_(filter) {

    init();
}

Reader::Reader(eckit::DataHandle& h, bool opened, const ReaderFilter& filter):
    handle_(&h),
    opened_(opened),
    filter_(filter) {

    init();

}

Reader::Reader(const eckit::PathName& path, const ReaderFilter& filter):
    handle_(path.fileHandle()),
    opened_(false),
    filter_(filter) {

    init();

}

void Reader::init() {
    if (!opened_) {
        handle_.openForRead();
    }

    std::cout << "PEEK "
              << handle_.peek(0)
              << handle_.peek(1)
              << handle_.peek(2)
              << handle_.peek(3)
              << std::endl;


    file_ = handle_.openf();
}

Reader::~Reader() {
    if (file_) {
        fclose(file_);
    }

    if (!opened_) {
        handle_.close();
    }
}

Message Reader::next() {
    for (;;) {
        int err = 0;
        codes_handle* h = codes_handle_new_from_file(nullptr, file_, PRODUCT_ANY, &err);
        ASSERT(err == 0);
        Message msg{h};
        if (!msg or filter_(msg)) {
            return msg;
        }
    }
}

void Reader::print(std::ostream &s) const {
    s << "Reader[" << handle_ << "]";
}

eckit::Offset Reader::position() {
    return handle_.position();
}


}  // namespace codes
}  // namespace metkit

