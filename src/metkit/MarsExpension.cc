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

#include "metkit/MarsExpension.h"

using namespace eckit;

namespace metkit {

MarsExpension::MarsExpension() {

}

//----------------------------------------------------------------------------------------------------------------------
std::vector<MarsRequest> MarsExpension::operator()(const std::vector<MarsRequest>& requests) {
    std::vector<MarsRequest> result(requests);

    // Implement inheritence
    for (auto j = result.begin(); j != result.end(); ++j) {
        MarsRequest& r = (*j);

        MarsRequest::Params& language = languages_[r.name()];

        for (auto k = language.begin(); k != language.end(); ++k) {
            const std::string& name = (*k).first;
            std::vector<std::string> values;
            if (r.getValues(name, values) == 0) {

                const auto& inherited = (*k).second;
                r.setValues(name, std::vector<std::string>(inherited.begin(), inherited.end()));
            }
        }

        std::vector<std::string> params;
        r.getParams(params);
        for (auto k = params.begin(); k != params.end(); ++k) {
            std::vector<std::string> values;
            r.getValues(*k, values);
            language[*k] = std::list<std::string>(values.begin(), values.end());
        }
    }

    return result;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit
