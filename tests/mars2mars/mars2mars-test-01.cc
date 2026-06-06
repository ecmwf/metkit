/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <exception>
#include <iostream>
#include <string>

#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/MarsExpansion.h"

#include "metkit/mars2mars/api/Mars2Mars.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_mars_request.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace {

metkit::mars::MarsRequest generateMarsRequest() {
    metkit::mars::MarsRequest mars;
    mars.verb("retrieve");

    // String-only keys
    mars.setValue("class", "od");
    mars.setValue("stream", "oper");
    mars.setValue("type", "fc");
    mars.setValue("expver", "1");


    mars.setValue("date", "2026-02-09");
    mars.setValue("time", "00:00:00");

    mars.setValue("levtype", "sfc");

    // Explicit scalar conversions supported by the MarsRequest trait
    mars.setValue("param", "261018");
    mars.setValue("step", "0");

    metkit::mars::MarsExpansion expand(true, true);
    mars = expand.expand(mars);

    return mars;
}

}  // namespace

CASE("mars2mars_marsrequest_convert_to_json") {
    using metkit::mars2mars::Mars2Mars;
    using metkit::mars2mars::utils::dict_traits::dict_to_json;

    try {

        metkit::mars::MarsRequest marsRequest = generateMarsRequest();

        const auto [converted, misc ] =
            Mars2Mars{}.convert(marsRequest);

        const std::string json = dict_to_json(converted);

        std::cout << json << std::endl;

        EXPECT(!json.empty());
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Test failed with exception: ") + e.what());
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}