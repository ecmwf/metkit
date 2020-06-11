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

Reader::Reader(eckit::DataHandle* h, bool opened):
    HandleHolder(h),
    opened_(opened) {

    init();
}

Reader::Reader(eckit::DataHandle& h, bool opened):
    HandleHolder(&h),
    opened_(opened) {

    init();

}

Reader::Reader(const eckit::PathName& path):
    HandleHolder(path.fileHandle()),
    opened_(false) {

    init();

}

void Reader::init() {
    if (!opened_) {
        handle().openForRead();
    }
    file_ = handle().openf();
}

Reader::~Reader() {
    if (file_) {
        fclose(file_);
    }

    if (!opened_) {
        handle().close();
    }
}

Message Reader::next() {
    int err = 0;
    codes_handle* h = codes_handle_new_from_file(nullptr, file_, PRODUCT_GRIB, &err);
    ASSERT(err == 0);
    return Message(h);
}

void Reader::print(std::ostream &s) const {
    s << "Reader[" << handle() << "]";
}

eckit::Offset Reader::position() {
    return handle().position();
}


}  // namespace codes
}  // namespace metkit

