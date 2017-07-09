/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "eckit/exception/Exceptions.h"
#include "eckit/types/Types.h"

#include "metkit/netcdf/GridSpec.h"

// #include "metkit/netcdf/Attribute.h"
// #include "metkit/netcdf/Dimension.h"
// #include "metkit/netcdf/Exceptions.h"
#include "metkit/netcdf/Variable.h"
// #include "metkit/netcdf/HyperCube.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include <iostream>
#include <sstream>


namespace metkit {
namespace netcdf {

static eckit::Mutex* local_mutex = 0;
static std::map<size_t, GridSpecGuesser*>* m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<size_t, GridSpecGuesser*>();
}


GridSpec::GridSpec(const Variable &variable):
    variable_(variable) {
}

GridSpec::~GridSpec()
{
}

GridSpec* GridSpec::create(const Variable &variable) {

    GridSpec* spec = GridSpecGuesser::guess(variable);

    if (!spec) {
        std::ostringstream oss;
        oss <<  "Cannot guess GridSpec for " << variable;
        throw eckit::SeriousBug(oss.str());
    }

    return spec;

}


//================================================================

GridSpecGuesser::GridSpecGuesser(size_t priority) :
    priority_(priority) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(priority) == m->end());
    (*m)[priority] = this;
}

GridSpecGuesser::~GridSpecGuesser() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(priority_);
}

GridSpec* GridSpecGuesser::guess(const Variable &variable) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<size_t, GridSpecGuesser*>::const_iterator j;

    const Variable &latitudes = variable.coordinateByAttribute("standard_name", "latitude");
    const Variable &longitudes = variable.coordinateByAttribute("standard_name", "longitude");


    for (j = m->begin(); j != m->end(); ++j) {
        GridSpec* spec = (*j).second->guess(variable, latitudes, longitudes);
        if (spec) {
            return spec;
        }
    }

    return 0;

}

}
}
