/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <unistd.h>
#include <pthread.h>

#include "marskit/ClientTask.h"
#include "marskit/MarsHandle.h"

namespace marskit {

// Call by the clien code
ClientTask::ClientTask(const MarsRequest &r, const MarsRequest &e, const std::string &host, int port)
    : request_(r),
      environ_(e),
      port_(port),
      host_(host),
      handle_(0) {
    // Try something unique (per machine)
    typedef unsigned long long ull;
    marskitID_ = (ull(::getpid()) << 32 )
               | (ull(::pthread_self()) << 16)
               | (ull(::time(0)) & ull(0xffff));

    handle_   = std::auto_ptr<eckit::DataHandle>(new MarsHandle(host_, port_, marskitID_));
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
    s << marskitID_;

    /* Send datahandle */

    s << *handle_;

    s.endObject();
}


char ClientTask::receive(eckit::Stream &s) const 
{
    unsigned long long id;
    char mode;

    s >> id; ASSERT(id == marskitID_);
    s >> mode;

    return mode;
}

}
