/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File ClientTask.h
// Baudouin Raoult - ECMWF Oct 96

#ifndef metkit_ClientTask_H
#define metkit_ClientTask_H

#include <memory>

// #include "eckit/bases/Watcher.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
// #include "eckit/log/UserChannel.h"
#include "eckit/transaction/TxnEvent.h"

#include "metkit/mars/MarsRequest.h"

namespace metkit {
namespace mars {

class ClientTask {
public:


    // -- Contructors

    ClientTask(const MarsRequest&, const MarsRequest&, const std::string& name, int port, unsigned long long id = 0);

    // ClientTask(eckit::Stream&);

    // -- Destructor

    ~ClientTask();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    // void dump(std::ostream&) const;
    // void json(eckit::JSON&) const;

    // MarsInfo info() const;
    // const MarsID&  id() const { return transactionID();  }
    // const MarsRequest& request()   const { return request_; }
    // const MarsRequest& environ()   const { return environ_; }

    // eckit::Length transferData(const eckit::PathName&);
    // eckit::PathName transferData();

    //    /// @returns eckit::DataHandle with the data that will be received, but does not give ownership of the
    //    eckit::DataHandle eckit::DataHandle& getDataHandle();

    // void sendData(eckit::DataHandle&);
    // void sendHandle(eckit::DataHandle&);

    // void acknowledge(eckit::Stream& s)     { send(s,'a'); }
    // void patch();
    //    bool authenticated() const;

    // eckit::DataHandle& dataHandle() { return *handle_; }

    // // Report

    // void success();
    // void failure(const std::string&);
    // void retry(const std::string&);

    // void infoMsg(const std::string&);
    // void warningMsg(const std::string&);
    // void errorMsg(const std::string&);
    // void notifyClient(const std::string&);
    // void notifyStart();

    // void   queueTime();

    // void ping();
    // void sendCost();

    // void sendChecksum(const std::string&);

    // // Queuing

    // double startingPriority()  const  { return startingPriority_;}
    // void   startingPriority(double p) { startingPriority_ = p;}

    // // Cost

    // Cost& cost()             { return cost_; }
    // const Cost& cost() const { return cost_; }
    // void  costChanged();


    //    // Called by PipeProcess

    //    void           send(eckit::Stream&);
    //    void           reply(eckit::Stream&);
    // bool           error(std::exception&,int);
    // void           done();

    // Logging


    // Mars tree

    // void push(const std::string&,const std::string&);
    // void pop();
    // void reset();

    // const eckit::StringList& treeNames() const { return treeNames_; }
    // const eckit::StringList& treeValues() const { return treeValues_; }

    //    // For the metkit
    void send(eckit::Stream&) const;
    char receive(eckit::Stream&) const;


    // -- Overridden methods

    // From Streamble

    // virtual void encode(eckit::Stream&) const override;
    // virtual const eckit::ReanimatorBase& reanimator() const { return reanimator_; }

    // From watcher

    // virtual void watch();

    // -- Class methods

    // static const eckit::ClassSpec&  classSpec()        { return classSpec_;}
    // static void  recover(TxnRecoverer<ClientTask>&);
    // static void  find(TxnFinder<ClientTask>&);
    // static std::string commandName();

    // None

protected:

    // -- Members

    MarsRequest request_;
    MarsRequest environ_;

private:

    // -- Members

    // unsigned long long   txnID_; // unused
    unsigned long long metkitID_;
    int port_;
    std::string host_;
    std::unique_ptr<eckit::DataHandle> handle_;
    std::string checksum_;

    // Not sent over streams

    // double               startingPriority_;
    // Cost                 cost_;
    // time_t               lastPing_;
    // time_t               queueTime_;

    // eckit::StringList			treeValues_;
    // eckit::StringList			treeNames_;

    //    bool                authenticated_;

    // -- Methods

    // void print(std::ostream&) const;
    // void send(eckit::Stream&,char) const;

    // -- Overridden methods
    // None

    // -- Class members

    // static eckit::ClassSpec               classSpec_;
    // static eckit::Reanimator<ClientTask>    reanimator_;

    // -- Class methods
    // None

    // friend std::ostream& operator<<(std::ostream& s, const ClientTask& r)
    //     { r.print(s); return s; }

    // friend eckit::JSON& operator<<(eckit::JSON& s, const ClientTask& r)
    //     { r.json(s); return s; }
};

}  // namespace mars
}  // namespace metkit

#endif
