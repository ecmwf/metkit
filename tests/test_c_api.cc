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
#include "eckit/types/Date.h"
#include <cstring>

using namespace eckit::testing;
namespace metkit::test {

// Wrapper around the C function calls that will throw an exception if the function fails
// i.e. if the return value is not METKIT_SUCCESS

void METKIT_TEST_C(int e) {
    metkit_error_values_t err = static_cast<metkit_error_values_t>(e);
    if (err != METKIT_SUCCESS)
        throw TestException("C-API error: " + std::string(metkit_get_error_string(err)), Here());
}

void EXPECT_STR_EQUAL(const char* a, const char* b) {
    if (strcmp(a, b) != 0)
        throw TestException("Expected: " + std::string(a) + " == " + std::string(b), Here());
}

// Fairly minimal test coverage
CASE( "metkit_marsrequest" ) {

    // -----------------------------------------------------------------
    // Basics
    // -----------------------------------------------------------------

    metkit_marsrequest_t* request{};

    METKIT_TEST_C(metkit_new_marsrequest(&request));
    EXPECT(request);

    // set/get verb
    METKIT_TEST_C(metkit_marsrequest_set_verb(request, "retrieve"));
    const char* verb{};
    METKIT_TEST_C(metkit_marsrequest_verb(request, &verb));
    EXPECT_STR_EQUAL(verb, "retrieve");

    // set array of values
    const char* dates[] = {"20200101", "20200102", "-1"};
    METKIT_TEST_C(metkit_marsrequest_set(request, "date", dates, 3));

    // set single value
    const char* expver = "xxxx";
    METKIT_TEST_C(metkit_marsrequest_set_one(request, "expver", expver));
    METKIT_TEST_C(metkit_marsrequest_set_one(request, "param", "2t"));

    // check values
    bool has = false;
    METKIT_TEST_C(metkit_marsrequest_has_param(request, "date", &has));
    EXPECT(has);

    METKIT_TEST_C(metkit_marsrequest_has_param(request, "random", &has));
    EXPECT(!has);

    size_t count = 0;
    METKIT_TEST_C(metkit_marsrequest_count_values(request, "date", &count));
    EXPECT_EQUAL(count, 3);

    for (size_t i = 0; i < count; i++) {
        const char* value{};
        METKIT_TEST_C(metkit_marsrequest_value(request, "date", i, &value));
        EXPECT_STR_EQUAL(value, dates[i]);
    }

    // all values 
    const char** values{};
    count = 0;
    METKIT_TEST_C(metkit_marsrequest_values(request, "date", &values, &count));
    EXPECT_EQUAL(count, 3);

    for (size_t i = 0; i < count; i++) {
        EXPECT_STR_EQUAL(values[i], dates[i]);
    }

    // -----------------------------------------------------------------
    // Expand
    // -----------------------------------------------------------------

    metkit_marsrequest_t* expandedRequest{};
    METKIT_TEST_C(metkit_new_marsrequest(&expandedRequest));
    METKIT_TEST_C(metkit_marsrequest_expand(request, expandedRequest, false, true));

    // Check date expanded -1 -> yesterday
    const char** dates_expanded{};
    METKIT_TEST_C(metkit_marsrequest_values(expandedRequest, "date", &dates_expanded, &count));
    EXPECT_EQUAL(count, 3);
    EXPECT_STR_EQUAL(dates_expanded[2], std::to_string(eckit::Date(-1).yyyymmdd()).c_str());
    // check param expanded 2t -> 167
    const char* param{};
    METKIT_TEST_C(metkit_marsrequest_value(expandedRequest, "param", 0, &param));
    EXPECT_STR_EQUAL(param, "167");
    
    // -----------------------------------------------------------------
    // Merge
    // -----------------------------------------------------------------

    metkit_marsrequest_t* req_manydates{};
    METKIT_TEST_C(metkit_new_marsrequest(&req_manydates));
    const char* dates_many[] = {"19000101", "19000102", "19000103"};
    METKIT_TEST_C(metkit_marsrequest_set(req_manydates, "date", dates_many, 3));

    METKIT_TEST_C(metkit_marsrequest_merge(request, req_manydates));
    METKIT_TEST_C(metkit_marsrequest_count_values(request, "date", &count));
    EXPECT_EQUAL(count, 6);

    // -----------------------------------------------------------------
    // done

    metkit_delete_marsrequest(request);
    metkit_delete_marsrequest(expandedRequest);
    metkit_delete_marsrequest(req_manydates);
}
//-----------------------------------------------------------------------------

CASE( "metkit_requestiterator_t parsing" ) {
    
    metkit_marsrequest_t* request0{};
    metkit_marsrequest_t* request1{};
    metkit_marsrequest_t* request2{};
    METKIT_TEST_C(metkit_new_marsrequest(&request0));
    METKIT_TEST_C(metkit_new_marsrequest(&request1));
    METKIT_TEST_C(metkit_new_marsrequest(&request2));

    metkit_requestiterator_t* it{};
    METKIT_TEST_C(metkit_parse_marsrequest("retrieve,date=-1,param=2t \n retrieve,date=20200102,param=2t,step=10/to/20/by/2", &it, true)); // two requests

    METKIT_TEST_C(metkit_requestiterator_request(it, request0));
    METKIT_TEST_C(metkit_requestiterator_next(it));
    METKIT_TEST_C(metkit_requestiterator_request(it, request1));

    EXPECT_EQUAL(metkit_requestiterator_next(it), METKIT_ITERATION_COMPLETE);
    EXPECT_NOT_EQUAL(metkit_requestiterator_request(it, request2), METKIT_SUCCESS); // Error to get request after iteration complete
    
    // check the date
    const char* date{};
    METKIT_TEST_C(metkit_marsrequest_value(request0, "date", 0, &date));
    EXPECT_STR_EQUAL(date, std::to_string(eckit::Date(-1).yyyymmdd()).c_str()); // parser also calls expand

    METKIT_TEST_C(metkit_marsrequest_value(request1, "date", 0, &date));
    EXPECT_STR_EQUAL(date, "20200102");

    // Check steps have been parsed
    const char** steps{};
    size_t count = 0;
    METKIT_TEST_C(metkit_marsrequest_values(request1, "step", &steps, &count));
    for (size_t i = 0; i < count; i++) {
        EXPECT_STR_EQUAL(steps[i], std::to_string(10 + i*2).c_str());
    }

}

}  // namespace metkit::test

int main(int argc, char **argv) {
    return run_tests ( argc, argv );
}
