/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_c_api.c
/// @date   Dec 2024
/// @author Christopher Bradley

#include "metkit/api/metkit_c.h"
#include <stdio.h>

int main(int argc, char **argv) {

    const char* version = metkit_version();

    metkit_error_t err = metkit_initialise();

    fprintf(stdout, "MetKit version: %s\n", version);

    return 0;
}
