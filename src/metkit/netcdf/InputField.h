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

#ifndef InputField_H
#define InputField_H

#include "Field.h"

class NCFileCache;

class InputField : public Field {
public:

    InputField(const std::string &, NCFileCache &);
    virtual ~InputField();

private:

    InputField(const InputField &);
    InputField &operator=(const InputField &);

    // -- Members

    int number_of_dimensions_;
    int number_of_variables_;
    int number_of_global_attributes_;
    int id_of_unlimited_dimension_;

    int format_;
    NCFileCache &cache_;

    // - Methods

    void print(std::ostream &s) const;

};

#endif
