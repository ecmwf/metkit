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

#include "metkit/netcdf/Variable.h"

#include "metkit/netcdf/Attribute.h"
#include "metkit/netcdf/Dimension.h"
#include "metkit/netcdf/Exceptions.h"
#include "metkit/netcdf/Dataset.h"
#include "metkit/netcdf/Matrix.h"
#include "metkit/netcdf/MergePlan.h"
#include "metkit/netcdf/Type.h"
#include "metkit/netcdf/Value.h"
#include "metkit/netcdf/GregorianDateCodec.h"

#include <iostream>

namespace metkit {
namespace netcdf {

static HyperCube::Dimensions cubedims(const std::vector<Dimension *> &dimensions) {
    HyperCube::Dimensions cdims;
    for (std::vector<Dimension *>::const_iterator j = dimensions.begin(); j != dimensions.end(); ++j) {
        cdims.push_back((*j)->count());
    }

    if (cdims.size() == 0) // Scalar variable
    {
        cdims.push_back(1);
    }

    return cdims;
}

Variable::Variable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions):
    owner_(owner),
    name_(name),
    matrix_(0),
    scalar_(dimensions.size() == 0),
    dimensions_(dimensions),
    cube_(cubedims(dimensions)),
    mustMerge_(false)
{
}

Variable::~Variable()
{
    if (matrix_) {
        matrix_->detach();
    }
}

void Variable::setMatrix(Matrix *matrix) {
    if (matrix) {
        matrix->attach();
    }
    if (matrix_) {
        matrix_->detach();
    }
    matrix_ = matrix;

    if (matrix_) {
        std::map<std::string, Attribute *>::const_iterator j = attributes_.find("_FillValue");
        std::map<std::string, Attribute *>::const_iterator k = attributes_.find("missing_value");
        if (j != attributes_.end() && k !=  attributes_.end()) {
            eckit::Log::warning() << "Variable '" << name() << "' has both 'missing_value' and '_FillValue' attributes" << std::endl;
            // throw MergeError(std::string("Variable ") + name() + " has both 'missing_value' and '_FillValue' attributes");
        }
        if (j == attributes_.end()) {
            j = k;
        }
        if (j != attributes_.end()) {
            matrix_->missingValue((*j).second->value().clone());
        }
    }
}

size_t Variable::numberOfValues() const {
    size_t count = 1;
    for (std::vector<Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j)
    {
        count *= (*j)->count();
    }
    return count;
}

std::vector<std::string> Variable::coordinates() const {
    std::vector<std::string> result;
    std::map<std::string, Attribute *>::const_iterator j = attributes_.find("coordinates");
    if (j != attributes_.end()) {
        std::string s = (*j).second->asString();
        std::string t;

        for (std::string::const_iterator k = s.begin(); k != s.end() ; ++k) {
            if (*k == ' ') {
                result.push_back(t);
                t.clear();
            }
            else {
                t.push_back(*k);
            }
        }

        if (t.size()) {
            result.push_back(t);
        }
    }
    return result;
}

std::vector<std::string> Variable::cellMethods() const {
    std::vector<std::string> result;
    std::map<std::string, Attribute *>::const_iterator j = attributes_.find("bounds");
    if (j != attributes_.end()) {
        std::string s = (*j).second->asString();
        result.push_back(s);
    }
    return result;
}

void Variable::dump(std::ostream &out) const
{
    out << "\t" ;
    if (matrix_) {
        matrix_->type().dump(out);
    }
    else {
        out << "unknown";
    }
    out << " " << name_ ;

    if (dimensions_.size()) {
        std::string sep = "(";
        for (std::vector<Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j)
        {
            out << sep << (*j)->name();
            sep = ", ";
        }
        out << ")";
    }
    out << " ;" << std::endl;

    for (std::map<std::string, Attribute *>::const_iterator j = attributes_.begin(); j != attributes_.end(); ++j)
    {
        (*j).second->dump(out);
    }
}

void Variable::dumpData(std::ostream &out) const {
    out << " " << name_ << " = " << std::endl;
    matrix_->dump(out);
    out << "  ;" << std::endl;
}

bool Variable::sameAs(const Variable &other) const {

    if (dummy()) {
        return sameAsDummy(other);
    }

    if (other.dummy()) {
        return other.sameAsDummy(*this);
    }

    return ncname() == other.ncname();
}

const std::string &Variable::path() const {
    return owner_.path();
}

const std::string &Variable::name() const {
    return name_;
}

const std::vector<Dimension *> &Variable::dimensions() const {
    return dimensions_;
}

bool Variable::scalar() const {
    return scalar_;
}

Matrix *Variable::matrix() const {
    if (!matrix_) {
        std::cout << "Variable::matrix " << *this << std::endl;
    }
    ASSERT(matrix_ != 0);
    return matrix_;
}

bool Variable::coordinate() const {
    // A coordinate variable as the same name as its unique dimension
    return (dimensions_.size() == 1 && dimensions_[0]->name() == name_);
}

HyperCube &Variable::cube()  {
    return cube_;
}

const HyperCube &Variable::cube() const {
    return cube_;
}

bool Variable::mustMerge() const {
    return mustMerge_;
}

void Variable::mustMerge(bool on) {
    mustMerge_ = on;
}

void Variable::resetCube() {
    cube_ = HyperCube(cubedims(dimensions_));
}

void Variable::addVirtualDimension(size_t where, Dimension *dim) {
    where = std::min(where, dimensions_.size());
    dimensions_.insert(dimensions_.begin() + where, dim);
    resetCube();
}

Dimension *Variable::getVirtualDimension() {
    std::cout << "Variable::getVirtualDimension: " << *this << std::endl;
    NOTIMP;
    return 0;
}

/*
See http://www.unidata.ucar.edu/software/netcdf/docs/netcdf/Attribute-Conventions.html
*/

static const char *not_supported[] = {
    "scale_factor",
    "add_offset",
    "signedness",
    "valid_range",
    0
};


void Variable::validate() const {
    size_t i = 0;
    while (not_supported[i]) {
        std::map<std::string, Attribute *>::const_iterator j = attributes_.find(not_supported[i]);
        if (j != attributes_.end()) {
            throw MergeError(std::string("Variable ") + name_ + " has an unsupported attribute: " + not_supported[i]);
        }
        i++;
    }
}

void Variable::create(int nc) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Variable::save(int nc) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

Variable *Variable::clone(Dataset &owner) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return 0;
}

void Variable::merge(const Variable &other, MergePlan &plan) {
    plan.link(*this, other);
    mergeAttributes(other);
}

Variable *Variable::makeDataVariable() {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return 0;
}

Variable *Variable::makeCoordinateVariable() {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return 0;
}

Variable *Variable::makeSimpleVariable() {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return 0;
}

Variable *Variable::makeCellMethodVariable() {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return 0;
}

Variable *Variable::makeScalarCoordinateVariable() {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return 0;
}

void Variable::initCodecs() {
    std::map<std::string, Attribute *>::const_iterator j = attributes_.find("units");
    if (j != attributes_.end()) {
        std::string units = (*j).second->asString();
        if (units.find("seconds since ") == 0) {
            std::string calendar = "gregorian";
            std::map<std::string, Attribute *>::const_iterator k = attributes_.find("calendar");
            if (k != attributes_.end()) {
                calendar = (*k).second->asString();
                if (calendar != "gregorian") {
                    throw MergeError(std::string("Unsupported calendar: ") + calendar);
                }
            }
            matrix()->codec(new GregorianDateCodec(units, calendar));
        }
    }
}


bool Variable::dummy() const {
    return false;
}

bool Variable::sameAsDummy(const Variable &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return false;
}

const std::string &Variable::ncname() const {
    return name_;
}

bool Variable::sharesDimensions(const Variable &other) const {
    for (std::vector<Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j)
    {
        for (std::vector<Dimension *>::const_iterator k = other.dimensions_.begin(); k != other.dimensions_.end(); ++k)
        {
            if (*j == *k) {
                return true;
            }
        }
    }
    return false;
}

bool Variable::timeAxis() const {
    return (matrix_->codec() && matrix_->codec()->timeAxis());
}

void Variable::collectField(std::vector<Field *>&) const {
    // Ignore
}



}
}
