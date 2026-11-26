// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File ClientTask.h
// Baudouin Raoult - ECMWF Oct 96

#pragma once

#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/transaction/TxnEvent.h"

#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

class ClientTask {
public:

    ClientTask(const MarsRequest&, const MarsRequest&, const std::string& name, int port, unsigned long long id = 0);

    ~ClientTask();

    void send(eckit::Stream&) const;
    char receive(eckit::Stream&) const;

protected:

    MarsRequest request_;
    MarsRequest environ_;

private:

    unsigned long long metkitID_;
    int port_;
    std::string host_;
    std::unique_ptr<eckit::DataHandle> handle_;
    std::string checksum_;
};

}  // namespace metkit::mars
