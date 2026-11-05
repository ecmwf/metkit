// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   test_c_api.c
/// @date   Dec 2024
/// @author Christopher Bradley

#include <stdio.h>
#include "metkit/api/metkit_c.h"

int main(int argc, char** argv) {

    const char* version = metkit_version();

    metkit_error_t err = metkit_initialise();

    fprintf(stdout, "MetKit version: %s\n", version);

    return 0;
}
