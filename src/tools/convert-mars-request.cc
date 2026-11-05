// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "ParseRequest.h"

int main(int argc, char** argv) {
    ParseRequest tool(argc, argv, true);
    return tool.start();
}
