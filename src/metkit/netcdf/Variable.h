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

#ifndef metkit_netcdf_Variable
#define metkit_netcdf_Variable

#include <string>
#include <vector>

#include "metkit/netcdf/Endowed.h"
#include "metkit/netcdf/HyperCube.h"


namespace metkit{
namespace netcdf{

class Dataset;
class Type;
class Attribute;
class Dimension;
class Matrix;
class MergePlan;
class HyperCube;
class Field;


class Variable : public Endowed {
public:
    Variable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions);
    virtual ~Variable();

    // -- Methods

    void setMatrix(Matrix *);
    size_t numberOfValues() const;
    std::vector<std::string> coordinates() const;
    std::vector<std::string> cellMethods() const;

    bool coordinate() const;
    bool scalar() const;


    virtual void validate() const;
    virtual bool sameAs(const Variable &) const;
    virtual void dump(std::ostream &s) const;
    virtual void dumpData(std::ostream &s) const;

    virtual void create(int nc) const;
    virtual void save(int nc) const;
    virtual Variable *clone(Dataset &owner) const;
    virtual void merge(const Variable &, MergePlan &);

    virtual Variable *makeDataVariable();
    virtual Variable *makeCoordinateVariable();
    virtual Variable *makeSimpleVariable();
    virtual Variable *makeCellMethodVariable();
    virtual Variable *makeScalarCoordinateVariable();
    virtual void initCodecs();

    // From Endowed
    virtual const std::string &name() const;
    virtual const std::string &path() const;

    Matrix *matrix() const;

    const std::vector<Dimension *> &dimensions() const;
    HyperCube &cube();
    const HyperCube &cube() const;

    bool mustMerge() const;
    void mustMerge(bool);

    virtual bool dummy() const;
    virtual bool sameAsDummy(const Variable &) const;

    virtual bool timeAxis() const;

    virtual void addVirtualDimension(size_t pos, Dimension *);
    virtual Dimension *getVirtualDimension();
    bool sharesDimensions(const Variable &other) const;

    virtual const std::string &ncname() const;
    virtual void collectField(std::vector<Field *>&) const;


protected:

    // -- Members

    Dataset &owner_;
    std::string name_;
    Matrix *matrix_;
    bool scalar_;

    std::vector<Dimension *> dimensions_;

    HyperCube::Remapping remapping_;
    HyperCube cube_;
    bool mustMerge_;

    void resetCube();

private:

    Variable(const Variable &);
    Variable &operator=(const Variable &);

    // -- Methods

    virtual void print(std::ostream &s) const = 0;

    // - Friend
    friend std::ostream &operator<<(std::ostream &s, const Variable &v)
    {
        v.print(s);
        return s;
    }
};

}
}
#endif
