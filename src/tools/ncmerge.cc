/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "metkit/netcdf/InputDataset.h"
#include "metkit/netcdf/NCFileCache.h"
#include "metkit/netcdf/OutputDataset.h"

#include <iostream>

using namespace metkit::netcdf;

int main(int argc, char **argv)
{
    try {

        NCFileCache cache;
        OutputDataset out("out.nc", cache);
        for (int i = 1 ; i < argc ; i++) {
            InputDataset f(argv[i], cache);
            std::cout << "@@@@@@@@@@@ " << argv[i] << std::endl;
            out.merge(f);
        }
        out.save();
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;

}

