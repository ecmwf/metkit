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

#ifndef metkit_netcdf_OutputField
#define metkit_netcdf_OutputField

#include "metkit/netcdf/Dataset.h"

namespace metkit{
namespace netcdf{

class NCFileCache;


class OutputDataset : public Dataset {
public:

    OutputDataset(const std::string &, NCFileCache &, int format = 0);
    virtual ~OutputDataset();

    // -- Methods

    void merge(Dataset &other);
    void save() const;

private:

    OutputDataset(const OutputDataset &);
    OutputDataset &operator=(const OutputDataset &);

    // -- Members

    int format_;
    NCFileCache &cache_;

    // - Methods

    // From Dataset
    virtual std::vector<const Field *> fields() const;

    virtual void print(std::ostream &s) const;

};

}
}
#endif
