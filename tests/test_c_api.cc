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
#include "metkit/mars/MarsRequest.h"
#include <cstring>

using namespace eckit::testing;
namespace metkit::test {

// Wrapper around the C function calls that will throw an exception if the function fails
// i.e. if the return value is not METKIT_SUCCESS

void test_succes(int e) {
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

    test_succes(metkit_marsrequest_new(&request));
    EXPECT(request);

    // set/get verb
    test_succes(metkit_marsrequest_set_verb(request, "retrieve"));
    const char* verb{};
    test_succes(metkit_marsrequest_verb(request, &verb));
    EXPECT_STR_EQUAL(verb, "retrieve");

    // set array of values
    const char* dates[] = {"20200101", "20200102", "-1"};
    test_succes(metkit_marsrequest_set(request, "date", dates, 3));

    // set single value
    const char* expver = "xxxx";
    test_succes(metkit_marsrequest_set_one(request, "expver", expver));
    test_succes(metkit_marsrequest_set_one(request, "param", "2t"));

    // check values
    bool has = false;
    test_succes(metkit_marsrequest_has_param(request, "date", &has));
    EXPECT(has);

    test_succes(metkit_marsrequest_has_param(request, "random", &has));
    EXPECT(!has);

    size_t count = 0;
    test_succes(metkit_marsrequest_count_values(request, "date", &count));
    EXPECT_EQUAL(count, 3);

    for (size_t i = 0; i < count; i++) {
        const char* value{};
        test_succes(metkit_marsrequest_value(request, "date", i, &value));
        EXPECT_STR_EQUAL(value, dates[i]);
    }

    // -----------------------------------------------------------------
    // Expand
    // -----------------------------------------------------------------

    metkit_marsrequest_t* expandedRequest{};
    test_succes(metkit_marsrequest_new(&expandedRequest));
    test_succes(metkit_marsrequest_expand(request, false, true, expandedRequest));

    // Check date expanded -1 -> yesterday
    test_succes(metkit_marsrequest_count_values(expandedRequest, "date", &count));
    EXPECT_EQUAL(count, 3);
    const char** dates_expanded = new const char*[count];
    for (size_t i = 0; i < count; i++) {
        test_succes(metkit_marsrequest_value(expandedRequest, "date", i, &dates_expanded[i]));
    }

    EXPECT_STR_EQUAL(dates_expanded[2], std::to_string(eckit::Date(-1).yyyymmdd()).c_str());
    // check param expanded 2t -> 167
    const char* param{};
    test_succes(metkit_marsrequest_value(expandedRequest, "param", 0, &param));
    EXPECT_STR_EQUAL(param, "167");
    
    // -----------------------------------------------------------------
    // Merge
    // -----------------------------------------------------------------

    metkit_marsrequest_t* req_manydates{};
    test_succes(metkit_marsrequest_new(&req_manydates));
    const char* dates_many[] = {"19000101", "19000102", "19000103"};
    test_succes(metkit_marsrequest_set(req_manydates, "date", dates_many, 3));

    test_succes(metkit_marsrequest_merge(request, req_manydates));
    test_succes(metkit_marsrequest_count_values(request, "date", &count));
    EXPECT_EQUAL(count, 6);

    // -----------------------------------------------------------------
    // done

    metkit_marsrequest_delete(request);
    metkit_marsrequest_delete(expandedRequest);
    metkit_marsrequest_delete(req_manydates);
}
//-----------------------------------------------------------------------------

CASE( "metkit_requestiterator_t parsing" ) {
    
    metkit_requestiterator_t* it{};
    test_succes(metkit_parse_marsrequests("retrieve,date=-1,param=2t \n retrieve,date=20200102,param=2t,step=10/to/20/by/2", &it, true)); // two separate requests

    std::vector <metkit_marsrequest_t*> requests;
    metkit_iterator_status_t status;
    while ((status = metkit_requestiterator_next(it)) == METKIT_ITERATOR_SUCCESS) {
        metkit_marsrequest_t* req{};
        test_succes(metkit_marsrequest_new(&req));
        EXPECT_EQUAL(metkit_requestiterator_current(it, req), METKIT_ITERATOR_SUCCESS);
        requests.push_back(req);
    }
    EXPECT_EQUAL(status, METKIT_ITERATOR_COMPLETE);
    EXPECT_EQUAL(requests.size(), 2);
    
    // check the date
    const char* date{};
    test_succes(metkit_marsrequest_value(requests[0], "date", 0, &date));
    EXPECT_STR_EQUAL(date, std::to_string(eckit::Date(-1).yyyymmdd()).c_str()); // parser also calls expand

    test_succes(metkit_marsrequest_value(requests[1], "date", 0, &date));
    EXPECT_STR_EQUAL(date, "20200102");

    // Check steps have been parsed
    size_t count = 0;
    test_succes(metkit_marsrequest_count_values(requests[1], "step", &count));
    EXPECT_EQUAL(count, 6);
    for (size_t i = 0; i < count; i++) {
        const char* step{};
        test_succes(metkit_marsrequest_value(requests[1], "step", i, &step));
        EXPECT_STR_EQUAL(step, std::to_string(10 + i*2).c_str());
    }

    // cleanup
    metkit_delete_requestiterator(it); // NB: requests have been moved out of the iterator
    for (auto req : requests) {
        metkit_marsrequest_delete(req);
    }
}

// Ensure that the param iterator works as expected
CASE( "metkit_paramiterator_t " ) {

    std::string str = "retrieve,date=20200102,param=2t,step=10/to/20/by/2";
    metkit_marsrequest_t* request{};
    test_succes(metkit_marsrequest_new(&request));
    test_succes(metkit_parse_marsrequest(str.c_str(), request, true));

    metkit_paramiterator_t* it{};
    test_succes(metkit_marsrequest_params(request, &it));
    metkit_iterator_status_t status;
    std::set<std::string> keys;
    while ((status = metkit_paramiterator_next(it)) == METKIT_ITERATOR_SUCCESS) {
        const char* key{};
        EXPECT_EQUAL(metkit_paramiterator_current(it, &key), METKIT_ITERATOR_SUCCESS);
        keys.insert(key);
    }
    EXPECT_EQUAL(status, METKIT_ITERATOR_COMPLETE);

    // Compare with C++ impl for consistency

    mars::MarsRequest req = mars::MarsRequest::parse(str, true);
    std::set<std::string> keys_cpp;
    for (const auto& k : req.params()) {
        keys_cpp.insert(k);
    }

    EXPECT_EQUAL(keys, keys_cpp);
}

CASE( "metkit_requestiterator_t 1 item" ) {
    // Edge case: verify iterator with one item works the same way.

    metkit_requestiterator_t* it{};
    test_succes(metkit_parse_marsrequests("retrieve,date=-1,param=2t", &it, true));

    std::vector <metkit_marsrequest_t*> requests;
    metkit_iterator_status_t status;
    while ((status = metkit_requestiterator_next(it)) == METKIT_ITERATOR_SUCCESS) {
        metkit_marsrequest_t* req{};
        test_succes(metkit_marsrequest_new(&req));
        EXPECT_EQUAL(metkit_requestiterator_current(it, req), METKIT_ITERATOR_SUCCESS);
        requests.push_back(req);
    }
    EXPECT_EQUAL(status, METKIT_ITERATOR_COMPLETE);
    EXPECT_EQUAL(requests.size(), 1);

    // cleanup
    metkit_delete_requestiterator(it);
    for (auto req : requests) {
        metkit_marsrequest_delete(req);
    }
}

}  // namespace metkit::test

int main(int argc, char **argv) {
    return run_tests ( argc, argv );
}
