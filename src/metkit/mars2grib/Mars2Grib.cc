/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/thread/AutoLock.h"

#include "metkit/config/LibMetkit.h"

#include "metkit/mars2grib/Mars2Grib.h"
#include "metkit/mars2grib/Rule.h"
#include "metkit/mars2grib/CodesKeySetter.h"


using eckit::Log;
using metkit::LibMetkit;

namespace {

static eckit::Mutex* local_mutex = 0;
static pthread_once_t once       = PTHREAD_ONCE_INIT;

static std::unique_ptr<metkit::mars2grib::RuleList> ruleList{nullptr};
}

static void init() {
    local_mutex = new eckit::Mutex();
    ruleList = std::make_unique<metkit::mars2grib::RuleList>(eckit::YAMLConfiguration{LibMetkit::mars2gribRuleListYamlFile()});
}

namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------


void convertMars2Grib(const eckit::ValueMap& initial, KeySetter& out) {
    pthread_once(&once, init);
    
    // TODO mutex not really required here?
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    
    ruleList->apply(initial, out);
}

void convertMars2Grib(const eckit::ValueMap& initial, grib::GribHandle& out) {
    CodesKeySetter ks{out};
    convertMars2Grib(initial, ks); 
}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2grib
