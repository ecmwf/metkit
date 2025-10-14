/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <set>

#include "eckit/config/Configuration.h"
#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/Resource.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/utils/MD5.h"
#include "eckit/utils/StringTools.h"
#include "eckit/utils/Translator.h"

#include "metkit/mars/MarsLocation.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------


MarsLocation::MarsLocation(const MarsRequest& r, const std::string& hostname, int port) :
    request_(r), hostname_(hostname), port_(port) {}

MarsLocation::MarsLocation(eckit::Stream& s) : request_(s) {
    s >> hostname_;
    s >> port_;

    NOTIMP;  // FIXME: added this constructor just to get develop branches to compile
}

MarsLocation::MarsLocation(const eckit::Configuration& c) :
    request_(c.getString("request")), hostname_(c.getString("server")), port_(c.getInt("port")) {}

MarsLocation::~MarsLocation() {}

metkit::mars::MarsLocation::operator eckit::Value() const {
    eckit::Value dict = eckit::Value::makeMap();

    dict["request"] = request_;
    dict["server"]  = hostname_;
    dict["port"]    = port_;

    return dict;
}

void MarsLocation::encode(eckit::Stream& s) const {
    s << request_;
    s << hostname_;
    s << port_;
}

const MarsRequest& MarsLocation::request() const {
    return request_;
}

std::string MarsLocation::hostname() const {
    return hostname_;
}

int MarsLocation::port() const {
    return port_;
}

void MarsLocation::print(std::ostream& s) const {
    s << request_ << ',' << std::endl;
    s << "hostname=" << hostname_ << ',' << std::endl;
    s << "port=" << port_ << std::endl;
}

void MarsLocation::json(eckit::JSON& s) const {
    s.startObject();
    s << "request" << request_;
    s << "hostname" << hostname_;
    s << "port" << port_;
    s.endObject();
}

//----------------------------------------------------------------------------------------------------------------------
}  // namespace mars
}  // namespace metkit
