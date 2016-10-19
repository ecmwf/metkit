/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <set>
#include <list>

#include "eckit/types/Types.h"
#include "eckit/parser/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/config/Resource.h"
#include "eckit/utils/Translator.h"
#include "eckit/utils/MD5.h"
#include "eckit/parser/StringTools.h"

#include "metkit/MarsLanguage.h"
#include "eckit/parser/JSONParser.h"

using namespace eckit;


static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {

    eckit::PathName language("~metkit/etc/language.json");


}



namespace metkit {

MarsLanguage::MarsLanguage(const std::string& verb) {
    pthread_once(&once, init);
}

//----------------------------------------------------------------------------------------------------------------------
void MarsLanguage::set(const std::string& name, const std::vector<std::string>& values) {
    language_[name] = values;
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
