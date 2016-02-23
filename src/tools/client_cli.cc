/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>

#include "eckit/io/FileHandle.h"
#include "eckit/runtime/Context.h"

#include "marskit/DHSProtocol.h"
#include "marskit/MarsRequestHandle.h"
#include "marskit/Client.h"



#include "marskit/MarsRequest.h"

#ifdef HAVE_ODB
#include "odb_api/ODBBehavior.h"
#include "odb_api/ODBModule.h"
#endif

using namespace std;

void test() {
    MarsRequest a("archive");
    a.setValue("class","od");
    a.setValue("expver","0001");
    a.setValue("type","fc");
    a.setValue("stream","oper");
    a.setValue("time","12");
    a.setValue("step","120");
    a.setValue("levtype","pl");
    a.setValue("levelist","1000");
    a.setValue("date","20111122");

    MarsRequest r("retrieve");
    r.setValue("class","od");
    r.setValue("expver","0001");
    r.setValue("type","fc");
    r.setValue("stream","oper");
    r.setValue("time","12");
    r.setValue("step","120");
    r.setValue("levtype","pl");
    r.setValue("levelist","1000");
    r.setValue("date","20111122");

    std::string host("localhost");

    // Archive
    {
        MarsRequestHandle h(a,new DHSProtocol("marsdev-core", host ,9000));
        eckit::FileHandle p("source.grib");
        p.saveInto(h);
    }

    // Retrieve
    {
        MarsRequestHandle h(r,new DHSProtocol("marsdev-core", host, 9000));
        eckit::FileHandle p("target.grib");
        h.saveInto(p);
    }

}

//=============================================================

int main(int argc,char **argv)
{

#ifdef HAVE_ODB
    Context::instance().behavior( new odb::ODBBehavior() );
    // TODO: enable $DEBUG (Log::debug)
#endif

    Client app(argc, argv);

#ifdef HAVE_ODB
    odb::ODBModule odbModule;
    app.executionContext().import(odbModule);
#endif
    app.start();
    return 0;
}
