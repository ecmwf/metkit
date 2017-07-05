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

#ifndef OutputField_H
#define OutputField_H

#include "Field.h"

class NCFileCache;

class OutputField : public Field {
public:

    OutputField(const std::string &, NCFileCache &, int format = 0);
    virtual ~OutputField();

    // -- Methods

    void merge(Field &other);
    void save() const;

private:

    OutputField(const OutputField &);
    OutputField &operator=(const OutputField &);

    // -- Members

    int format_;
    NCFileCache &cache_;

    // - Methods

    virtual void print(std::ostream &s) const;

};

#endif
