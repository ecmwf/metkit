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
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsRequest.h"

#include "metkit/mars2mars/api/Mars2Mars.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_mars_request.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"

#ifndef MARS2MARS_TEST_REQUEST_FILE
#define MARS2MARS_TEST_REQUEST_FILE "mars2mars-test-requests.mars"
#endif

namespace {

const std::vector<std::string>& mars2marsSplitKeys() {
    static const std::vector<std::string> keys = {
        "class",
        "stream",
        "type",
        "expver",
        "levtype",
        "param",
        "levelist",
        "step",
        "date",
        "time",
        "number",
        "chem",
        "wavelength",
        "timespan",
        "hdate",
        "htime",
    };

    return keys;
}

std::vector<metkit::mars::MarsRequest>
readRequestsFromFile(const std::string& path) {
    std::ifstream in(path);

    if (!in) {
        throw std::runtime_error("Cannot open MARS request file: " + path);
    }

    return metkit::mars::MarsRequest::parse(in);
}

std::vector<metkit::mars::MarsRequest>
flattenRequest(const metkit::mars::MarsRequest& request) {
    return request.split(mars2marsSplitKeys());
}

std::string jsonArrayFromRequests(
    const std::vector<metkit::mars::MarsRequest>& requests) {
    using metkit::mars2mars::utils::dict_traits::dict_to_json;

    std::ostringstream out;

    out << "[\n";

    for (std::size_t i = 0; i < requests.size(); ++i) {
        out << "  " << dict_to_json(requests[i]);

        if (i + 1 != requests.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "]";

    return out.str();
}

}  // namespace

CASE("mars2mars_flatten_convert_append_to_json") {
    using metkit::mars2mars::Mars2Mars;

    const std::string reqFileName = MARS2MARS_TEST_REQUEST_FILE;

    const std::vector<metkit::mars::MarsRequest> requests =
        readRequestsFromFile(reqFileName);

    EXPECT(!requests.empty());

    metkit::mars::MarsExpansion expand(true, true);
    Mars2Mars mars2mars;

    std::vector<metkit::mars::MarsRequest> convertedPoints;

    for (const auto& request : requests) {
        const metkit::mars::MarsRequest expanded = expand.expand(request);

        const std::vector<metkit::mars::MarsRequest> points =
            flattenRequest(expanded);

        EXPECT(!points.empty());

        for (const auto& point : points) {
            const auto [converted, misc] =
                mars2mars.convert(point);

            convertedPoints.push_back(converted);
        }
    }

    EXPECT(!convertedPoints.empty());

    const std::string outputJson = jsonArrayFromRequests(convertedPoints);

    std::cout << outputJson << std::endl;

    EXPECT(!outputJson.empty());
    EXPECT(outputJson.front() == '[');
    EXPECT(outputJson.back() == ']');
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}