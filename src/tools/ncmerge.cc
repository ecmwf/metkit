// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// Baudouin Raoult - ECMWF Jan 2015

#include "metkit/netcdf/InputDataset.h"
#include "metkit/netcdf/NCFileCache.h"
#include "metkit/netcdf/OutputDataset.h"

#include <iostream>

using namespace metkit::netcdf;

int main(int argc, char** argv) {
    try {

        NCFileCache cache;
        OutputDataset out("out.nc", cache);
        for (int i = 1; i < argc; i++) {
            InputDataset f(argv[i], cache);
            std::cout << "@@@@@@@@@@@ " << argv[i] << std::endl;
            out.merge(f);
        }
        out.save();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
