/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_c_api.cc
/// @date   Dec 2024
/// @author Christopher Bradley

#include "metkit/api/metkit_c.h"
#include "eckit/testing/Test.h"

using namespace eckit::testing;
namespace metkit::test {

CASE( "Request get/set" ) {

    metkit_request_t* request = nullptr;

    metkit_new_request(&request);
    EXPECT(request);

    // set/get verb
    metkit_request_set_verb(request, "retrieve");
    const char* verb = nullptr;
    metkit_request_verb(request, &verb);
    EXPECT(strcmp(verb, "retrieve") == 0);

    // set array of values
    const char* dates[] = {"20200101", "20200102", "20200103"};
    metkit_request_add(request, "date", dates, 3);

    // set single value
    const char* expver = "xxxx";
    metkit_request_add(request, "expver", &expver, 1);

    // check values
    bool has = false;
    metkit_request_has_param(request, "date", &has);
    EXPECT(has);

    metkit_request_has_param(request, "random", &has);
    EXPECT(!has);

    size_t count = 0;
    metkit_request_count_values(request, "date", &count);
    EXPECT_EQUAL(count, 3);

    for (size_t i = 0; i < count; i++) {
        const char* value = nullptr;
        metkit_request_value(request, "date", i, &value);
        EXPECT(strcmp(value, dates[i]) == 0);
    }

    // all values 
    const char** values = nullptr;
    count = 0;
    metkit_request_values(request, "date", &values, &count);
    EXPECT_EQUAL(count, 3);

    for (size_t i = 0; i < count; i++) {
        EXPECT(strcmp(values[i], dates[i]) == 0);
    }

    // done
    metkit_free_request(request);
}

//-----------------------------------------------------------------------------

}  // namespace metkit::test

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
