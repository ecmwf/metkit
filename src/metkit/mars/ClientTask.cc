/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <pthread.h>
#include <unistd.h>

#include "metkit/mars/ClientTask.h"
#include "metkit/mars/MarsHandle.h"

namespace metkit {
namespace mars {

// Call by the clien code
ClientTask::ClientTask(const MarsRequest& r, const MarsRequest& e, const std::string& host, int port,
                       unsigned long long id) :
    request_(r), environ_(e), metkitID_(id), port_(port), host_(host), handle_() {
    // Try something unique (per machine)
    // Warning: Servers recovers time(0) from ID
    // to compute request age. Not good
    if (!metkitID_) {
        typedef unsigned long long ull;
        metkitID_ =
            (ull(::getpid()) << ull(32 + 16)) | (ull(::pthread_self()) << ull(32)) | (ull(::time(0)) & ull(0xffffffff));
    }

    handle_.reset(new MarsHandle(host_, port_, metkitID_));
}


ClientTask::~ClientTask() {}

void ClientTask::send(eckit::Stream& s) const {
    unsigned long long dummy = 0;
    s.startObject();
    s << "MarsTask";

    /* send mars request id */
    s << dummy;

    /* Send requests */
    s << request_;
    s << environ_;

    /* Send cb info */
    s << host_;
    s << port_;
    s << metkitID_;

    /* Send datahandle */

    s << *handle_;

    s.endObject();
}


char ClientTask::receive(eckit::Stream& s) const {
    unsigned long long id;
    char mode;

    s >> id;
    ASSERT(id == metkitID_);
    s >> mode;

    return mode;
}

}  // namespace mars
}  // namespace metkit
