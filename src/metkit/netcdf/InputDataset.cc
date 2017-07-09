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

#include "metkit/netcdf/Exceptions.h"
#include "metkit/netcdf/InputDimension.h"
#include "metkit/netcdf/InputMatrix.h"
#include "metkit/netcdf/NCFile.h"
#include "metkit/netcdf/NCFileCache.h"
#include "metkit/netcdf/SimpleInputVariable.h"
#include "metkit/netcdf/Type.h"

#include <netcdf.h>

namespace metkit {
namespace netcdf {

InputDataset::InputDataset(const std::string &path, NCFileCache &cache):
    Dataset(path),
    cache_(cache)
{
    char name[NC_MAX_NAME + 1];

    NCFile &file = cache.lookUp(path_);
    int nc = file.open();

    NC_CALL(nc_inq(nc, &number_of_dimensions_, &number_of_variables_,
                   &number_of_global_attributes_,
                   &id_of_unlimited_dimension_), path_);

    NC_CALL(nc_inq_format(nc, &format_), path_);

    for (size_t i = 0; i < number_of_dimensions_; i++)
    {
        size_t count;
        NC_CALL(nc_inq_dim(nc, i, name, &count), path_);
        add(new InputDimension(*this, name, i, count));
    }

    for (size_t i = 0; i < number_of_variables_; i++)
    {
        int type;
        int ndims, nattr;
        int dims[NC_MAX_VAR_DIMS];

        NC_CALL(nc_inq_var(nc, i, name, &type, &ndims, dims, &nattr), path_);

        Type &kind = Type::lookup(type);

        std::vector<Dimension *> dimensions;
        for (size_t j = 0; j < ndims; j++) {
            dimensions.push_back(findDimension(dims[j]));
        }

        Variable *v = new SimpleInputVariable(*this, name, i, dimensions);
        v->setMatrix(new InputMatrix(kind, i, name, v->numberOfValues(), file));
        v->getAttributes(nc, i, nattr);
        add(v);
    }

    getAttributes(nc, NC_GLOBAL, number_of_global_attributes_);

    file.close();

    // Finalise...

    for (std::map<std::string, Variable *>::iterator j = variables_.begin(); j != variables_.end(); ++j)
    {
        Variable *v = (*j).second;

        if (v->coordinate()) {

            // This is a coordinate variable
            Variable *w = v->makeCoordinateVariable();
            if (w != v) {
                delete v;
                (*j).second = w;
                v = w;
            }

        }

        std::vector<std::string> coordinates = v->coordinates();
        std::vector<std::string> cellMethods = v->cellMethods();


        if (coordinates.size()) {

            // This is a data variable
            Variable *w = v->makeDataVariable();
            if (w != v) {
                delete v;
                (*j).second = w;
                v = w;
            }

            size_t i = 0;
            for (std::vector<std::string>::iterator k = coordinates.begin(); k != coordinates.end(); ++k, ++i)
            {
                // This is a coordinate variable
                std::map<std::string, Variable *>::iterator m = variables_.find(*k);
                if (m == variables_.end()) {
                    eckit::Log::error() << "Coordinate '" << *k << "' has no corresponding variable" << std::endl;
                    continue;
                }
                ASSERT(m != variables_.end());
                Variable *t = (*m).second;
                Variable *w = t->makeCoordinateVariable();
                if (w != t) {
                    delete t;
                    (*m).second = w;
                    t = w;
                }

                // Some coordinates are scalar
                if ((*m).second->scalar()) {
                    Variable *t = (*m).second;
                    Variable *w = t->makeScalarCoordinateVariable();
                    if (w != t) {
                        delete t;
                        (*m).second = w;
                        t = w;
                    }

                    (*j).second->addVirtualDimension(i, (*m).second->getVirtualDimension());
                }
                // Check if variable shares dimension with
		
                if(!(*j).second->sharesDimensions(*(*m).second)) {
eckit::Log::error() << *(*j).second << " must share dimensions with " << *(*m).second << std::endl;

}
                //ASSERT((*j).second->sharesDimensions(*(*m).second));
            }
        }

        if (cellMethods.size()) {

            for (std::vector<std::string>::iterator k = cellMethods.begin(); k != cellMethods.end(); ++k)
            {
                // This is a coordinate variable
                std::map<std::string, Variable *>::iterator m = variables_.find(*k);
                ASSERT(m != variables_.end());
                Variable *t = (*m).second;
                Variable *w = t->makeCellMethodVariable();
                if (w != t) {
                    delete t;
                    (*m).second = w;
                    t = w;
                }

            }
        }

        (*j).second->initCodecs();
    }


    for (std::map<std::string, Variable *>::iterator j = variables_.begin(); j != variables_.end(); ++j) {
        Variable *v = (*j).second;
        if (v->timeAxis()) {
            std::vector<std::string> cellMethods = v->cellMethods();

            for (std::vector<std::string>::iterator k = cellMethods.begin(); k != cellMethods.end(); ++k)
            {
                // This is a coordinate variable
                std::map<std::string, Variable *>::iterator m = variables_.find(*k);
                ASSERT(m != variables_.end());
                Variable *t = (*m).second;
                for (std::map<std::string, Variable *>::iterator j = variables_.begin(); j != variables_.end(); ++j) {
                    Variable *p = (*j).second;
                    if (p != v && p->timeAxis()) {
                        Dimension *d;
                        if (!p->scalar()) {
                            d = p->getVirtualDimension();
                        }
                        else {
                            ASSERT(p->dimensions().size() == 1);
                            d = p->dimensions()[0];
                        }
                        t->addVirtualDimension(0, d); // Add 'reftime' to 'time_bounds'
                        v->addVirtualDimension(0, d); // Add 'reftime' to 'time'
                    }
                }
            }
        }
    }

    for (std::map<std::string, Variable *>::iterator j = variables_.begin(); j != variables_.end(); ++j) {
        Variable *v = (*j).second;
        v->validate();
    }

}

InputDataset::~InputDataset()
{
}


void InputDataset::print(std::ostream &out) const
{
    out << "InputDataset[path=" << path_ << "]";
}


std::vector<Field *> InputDataset::fields() const {

    std::vector<Field *> result;

    for (std::map<std::string, Variable *>::const_iterator j = variables_.begin(); j != variables_.end(); ++j) {
        Variable *v = (*j).second;
        v->collectField(result);
    }

    return result;
}


}
}
