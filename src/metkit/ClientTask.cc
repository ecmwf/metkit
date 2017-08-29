/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <unistd.h>
#include <pthread.h>

#include "metkit/ClientTask.h"
#include "metkit/MarsHandle.h"

namespace metkit {

// Call by the clien code
ClientTask::ClientTask(const MarsRequest &r, const MarsRequest &e, const std::string &host, int port, unsigned long long id)
    : request_(r),
      environ_(e),
      port_(port),
      host_(host),
      handle_(0),
      metkitID_(id) {
    // Try something unique (per machine)
    if (!metkitID_) {
        typedef unsigned long long ull;
        metkitID_ = (ull(::getpid()) << 32 )
                    | (ull(::pthread_self()) << 16)
                    | (ull(::time(0)) & ull(0xffff));
    }

    handle_.reset(new MarsHandle(host_, port_, metkitID_));
}


ClientTask::~ClientTask() {}

void ClientTask::send(eckit::Stream &s) const
{
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


char ClientTask::receive(eckit::Stream &s) const
{
    unsigned long long id;
    char mode;

    s >> id; ASSERT(id == metkitID_);
    s >> mode;

    return mode;
}

}
